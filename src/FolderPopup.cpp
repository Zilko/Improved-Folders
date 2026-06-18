#include "Utils.hpp"
#include "FolderPopup.hpp"
#include "CreateFolderPopup.hpp"

FolderPopup* FolderPopup::create(SetFolderPopup* ogPopup, bool noElasticity) {
    FolderPopup* ret = new FolderPopup();

	ret->m_ogPopup = ogPopup;
	ret->m_noElasticity = noElasticity;

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

void FolderPopup::onClose(CCObject*) {
	setKeypadEnabled(false);
	setTouchEnabled(false);
	removeFromParent();

	if (!m_ogPopup) return;
	if (m_value == m_ogPopup->m_value) return m_ogPopup->onClose(nullptr);

	auto doModify = true;

	if (m_moveLevel) {
		if (m_deletingItself) {
			auto folder = getCurrentFolder();

			if (folder != m_moveLevel->m_levelFolder) {
				doModify = false;

				auto obj = Mod::get()->getSavedValue<matjson::Value>("levels");
				auto id = EditorIDs::getID(m_moveLevel);
				auto arr = obj[numToString(folder)];
				auto newArr = matjson::Value::array();

				for (const auto& v : arr) {
					if (v.asInt().unwrapOr(id + 1) != id) {
						newArr.push(v);
					}
				}

				obj[numToString(folder)] = newArr;

				Mod::get()->setSavedValue("levels", obj);
			}
		} else if (m_resetAll && m_value == 0) {
			auto obj = Mod::get()->getSavedValue<matjson::Value>("levels");
			auto id = EditorIDs::getID(m_moveLevel);

			for (const auto& v : obj) {
				auto key = v.getKey().value_or("");
				
				if (key.empty() || !v.isArray()) {
					continue;
				}

				auto newArr = matjson::Value::array();

				for (const auto& b : v) {
					if (b.asInt().unwrapOr(id + 1) != id) {
						newArr.push(b);
					}
				}
				
				obj[key] = newArr;
			}
			
			Mod::get()->setSavedValue("levels", obj);
		} else if (m_moveLevel->m_levelFolder != 0) {
			doModify = false;

			auto obj = Mod::get()->getSavedValue<matjson::Value>("levels");
			auto id = EditorIDs::getID(m_moveLevel);
			auto arr = obj[numToString(m_value)];

			if (!arr.isArray()) {
				arr = matjson::Value::array();
			}

			auto didContain = false;

			for (const auto& v : arr) {
				if (v.asInt().unwrapOr(id + 1) == id) {
					didContain = true;
					break;
				}
			}

			if (!didContain) {
				arr.push(EditorIDs::getID(m_moveLevel));
				obj[numToString(m_value)] = arr;
				Notification::create(fmt::format("Level moved to \"{}\"", Utils::getFolderName(m_value, m_searchType)))->show();
				Mod::get()->setSavedValue("levels", obj);
			}
		}
	}

	if (doModify) {
		m_ogPopup->m_value = m_value;
	}

	m_ogPopup->onClose(nullptr);

	if (m_isMove && !Mod::get()->getSettingValue<bool>("disable-notification")) {
		if (m_value == 0)
			Notification::create("Level removed from folder.")->show();
		else if (doModify)
			Notification::create(fmt::format("Level moved to \"{}\"", Utils::getFolderName(m_value, m_searchType)))->show();
	}
}

bool FolderPopup::init() {
	Popup::init(370, 267);

	m_isInverted = Mod::get()->getSavedValue<bool>("sort-inverted");
	m_isGrid = Mod::get()->getSavedValue<bool>("grid-enabled");

	if (m_ogPopup) {
		m_value = m_ogPopup->m_value;
		m_currentPath = Utils::getFolderPath(m_value, true, m_searchType);
		m_currentIndex = static_cast<int>(m_currentPath.size()) - 1;

		if (LevelBrowserLayer* layer = typeinfo_cast<LevelBrowserLayer*>(m_ogPopup->m_delegate))
			m_searchType = layer->m_searchObject->m_searchType;
		else {
			m_searchType = typeinfo_cast<EditLevelLayer*>(m_ogPopup->m_delegate) ? SearchType::MyLevels : SearchType::SavedLevels;
			m_isMove = true;

			if (EditLevelLayer* layer = typeinfo_cast<EditLevelLayer*>(m_ogPopup->m_delegate))
				m_moveLevel = layer->m_level;
			
			if (LevelInfoLayer* layer = typeinfo_cast<LevelInfoLayer*>(m_ogPopup->m_delegate))
				m_moveLevel = layer->m_level;
		}
	}

	setTitle(m_isMove ? "Move to Folder" : "Go to Folder");
    m_title->setPositionY(250);

	NineSlice* bg = NineSlice::create("square02b_001.png");
	bg->setColor({0, 0, 0});
	bg->setOpacity(90);
	bg->setContentSize({ 513, 60 });
    bg->setAnchorPoint({0, 0.5f});
	bg->setScale(0.3372f);
	bg->setPosition({39, 226});

	m_mainLayer->addChild(bg);

	m_pathLabel = CCLabelBMFont::create("Local Levels/", "bigFont.fnt");
    m_pathLabel->setAnchorPoint({0, 0.5f});
	m_pathLabel->setPosition({44, 226});
	m_pathLabel->setScale(0.225f);

	m_mainLayer->addChild(m_pathLabel);

	m_emptyLabel = CCLabelBMFont::create("Empty", "bigFont.fnt");
    m_emptyLabel->setOpacity(90);
	m_emptyLabel->setScale(0.425f);
	m_emptyLabel->setPosition(m_size / 2.f);

	m_mainLayer->addChild(m_emptyLabel, 2);

	m_searchInput = TextInput::create(167, "Search", "bigFont.fnt");
	m_searchInput->setPosition({330, 226});
    m_searchInput->setAnchorPoint({1, 0.5f});
	m_searchInput->setScale(0.675f);
	m_searchInput->getInputNode()->setDelegate(this);

	m_mainLayer->addChild(m_searchInput);

    CCSprite* spr = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
	spr->setScale(0.7f);
    m_addButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FolderPopup::onCreateFolder));
	m_addButton->setPosition({m_size.width - 29, 29});

	m_buttonMenu->addChild(m_addButton);

	spr = CCSprite::createWithSpriteFrameName("GJ_resetBtn_001.png");
	spr->setScale(1.25f);
    m_resetButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FolderPopup::onReset));
	m_resetButton->setPosition({m_size.width - 70, 29});
	m_resetButton->setVisible(m_isMove);

	m_buttonMenu->addChild(m_resetButton);

    spr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	spr->setScale(0.55f);
    m_leftArrow = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FolderPopup::onPrevious));
	m_leftArrow->setCascadeOpacityEnabled(true);
	m_leftArrow->setPosition({29, 29});

	m_buttonMenu->addChild(m_leftArrow);

	spr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
	spr->setScale(0.55f);
	spr->setFlipX(true);
    m_rightArrow = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FolderPopup::onNext));
	m_rightArrow->setCascadeOpacityEnabled(true);
	m_rightArrow->setPosition({67, 29});

	m_buttonMenu->addChild(m_rightArrow);

	m_sortToggle = CCMenuItemToggler::create(
		Utils::createTogglerSprite("GJ_sortIcon_001.png", false, true),
		Utils::createTogglerSprite("GJ_sortIcon_001.png", true, true),
		this,
		menu_selector(FolderPopup::onSortToggle)
	);
	m_sortToggle->setScale(0.47f);
	m_sortToggle->setPosition({(m_size.width - 289) / 4, 196});
	m_sortToggle->toggle(m_isInverted);
	m_buttonMenu->addChild(m_sortToggle);

	m_listToggle = CCMenuItemToggler::create(
		Utils::createTogglerSprite("GJ_extendedIcon_001.png", false, true),
		Utils::createTogglerSprite("GJ_extendedIcon_001.png", true, true),
		this,
		menu_selector(FolderPopup::onListToggle)
	);
	m_listToggle->setScale(0.47f);
	m_listToggle->setPosition({(m_size.width - 289) / 4, 168});
	m_listToggle->toggle(!m_isGrid);
	m_buttonMenu->addChild(m_listToggle);

	m_gridToggle = CCMenuItemToggler::create(
		Utils::createTogglerSprite("geode.loader/grid-view.png", false, true),
		Utils::createTogglerSprite("geode.loader/grid-view.png", true, true),
		this,
		menu_selector(FolderPopup::onGridToggle)
	);
	m_gridToggle->setScale(0.47f);
	m_gridToggle->setPosition({(m_size.width - 289) / 4, 140});
	m_gridToggle->toggle(m_isGrid);
	m_buttonMenu->addChild(m_gridToggle);

	spr = CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
	spr->setScale(0.6f);
	m_moveButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FolderPopup::onMove));
	m_moveButton->setPosition({291, 29});
	m_moveButton->setCascadeOpacityEnabled(true);
	m_moveButton->setVisible(false);

	m_buttonMenu->addChild(m_moveButton);

	spr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
	spr->setScale(0.425f);
	m_cancelMoveButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FolderPopup::onCancelMove));
	m_cancelMoveButton->setPosition({260, 29});
	m_cancelMoveButton->setVisible(false);

	m_buttonMenu->addChild(m_cancelMoveButton);

	m_movingLabel = CCLabelBMFont::create("Moving \"wa1\"", "bigFont.fnt");
	m_movingLabel->setPosition({277, 48});
	m_movingLabel->setOpacity(156);
	m_movingLabel->setVisible(false);

	m_mainLayer->addChild(m_movingLabel);

	spr = CCSprite::createWithSpriteFrameName("gj_findBtnOff_001.png");
	spr->setScale(0.575f);
	spr->setOpacity(165);
	m_searchOff = CCMenuItemSpriteExtra::create(spr, this, menu_selector(FolderPopup::onCancelSearch));
	m_searchOff->setPosition({343, 226});
	m_searchOff->setVisible(false);

	m_buttonMenu->addChild(m_searchOff);

	bg = NineSlice::create("square02b_001.png");
	bg->setColor(ccc3(138, 77, 46));
	bg->setContentSize({ 289, 153 });
    bg->setPosition((m_size - ccp(289, 153)) / 2.f);
    bg->setAnchorPoint({0, 0});
	m_mainLayer->addChild(bg, 1);

    updateList();

	if (!m_noElasticity) {
		runAction(CCSequence::create(
			CCDelayTime::create(0.05f),
			CCCallFunc::create(this, callfunc_selector(FolderPopup::updateTableView)),
			nullptr
		));
	}

    return true;
}

void FolderPopup::onCancelSearch(CCObject*) {
	m_searchInput->setString("");

	textChanged(nullptr);
}

void FolderPopup::onCancelMove(CCObject*) {
	if (m_movingNode) m_movingNode->enableMoveButton(true);

	m_isMoving = false;
	m_movingId = 0;
	m_movingNode = nullptr;
	m_movingItem = {};

	updateButtons();
}

void FolderPopup::onReset(CCObject*) {
	geode::createQuickPopup(
		"Warning",
		"Are you sure you want to <cr>remove</c> this level from its folder(s)?\n<cl>(You will still be able to see it from \"All levels\")</c>",
		"No", "Yes",
		[this](auto, bool yes) {
			if (!yes) return;

			m_resetAll = true;
			m_value = 0;
			onClose(nullptr);
		}
	);
}

void FolderPopup::textChanged(CCTextInputNode*) {
	m_search = m_searchInput->getString();
	m_searchOff->setVisible(!m_search.empty());
	
	m_folderPositions.erase(getCurrentFolder());

	updateList();
}

void FolderPopup::updateItems() {
	m_items.clear();

	if (m_currentPath.empty() || m_currentIndex < 0) {
		m_items = Utils::getAllFolders(m_isInverted, m_searchType);
		return;
	}

	m_items = Utils::getItemsForPath(m_currentPath, m_currentIndex, m_isInverted, m_searchType);
	
	for (Item& item : m_items) {
		if (item.type != ItemType::Level) continue;

		if (!m_levelIds.contains(item.level)) {
			int id = static_cast<int>(m_levelIds.size()) + 1;

			m_levelIds[item.level] = id;
			item.id = id;
		} else
			item.id = m_levelIds.at(item.level);
	}
}

void FolderPopup::updatePath() {
	std::string path = m_searchType == SearchType::MyLevels ? "Local Levels/" : "Saved Levels/";
	int rep = -1;

	do {
		for (int i = 0 + (rep > 0 ? rep : 0); i <= m_currentIndex; i++)
			path += Utils::getFolderName(m_currentPath[i], m_searchType) + "/";

		if (m_viewingLevel) path += m_viewingLevel->m_levelName;

		m_pathLabel->setString(path.c_str());

		path = ".../";
		rep++;

	} while (m_pathLabel->getContentSize().width > 722);

	if (std::string_view(m_pathLabel->getString()) == ".../") {
		std::string path = ".../" + Utils::getFolderName(m_currentPath.back(), m_searchType);

		do {
			for (int i = 0; i < 5; i++)
				path.pop_back();

			path += "...";
			path += m_viewingLevel ? "" : "/";

			m_pathLabel->setString(path.c_str());

		} while (m_pathLabel->getContentSize().width > 722);
	}

	path = m_pathLabel->getString();
	CCArrayExt<CCSprite*> children = CCArrayExt<CCSprite*>(m_pathLabel->getChildren());

	for (int i = 0; i < children.size(); i++)
		children[i]->setOpacity(path[i] == '/' ? 105 : 165);
}

void FolderPopup::keyDown(cocos2d::enumKeyCodes key, double timestamp) {
	if (key == cocos2d::enumKeyCodes::KEY_Escape)
		return m_currentIndex < 0 ? onClose(nullptr) : onPrevious(nullptr);

	if (key == cocos2d::enumKeyCodes::KEY_Q) return onPrevious(nullptr);

	if (key == cocos2d::enumKeyCodes::KEY_E) return onNext(nullptr);

	return FLAlertLayer::keyDown(key, timestamp);
	
}

ItemCell* FolderPopup::findMovingNode(ItemCell* cell) {
	if (!m_isMoving) return cell;

	for (ItemNode* node : CCArrayExt<ItemNode*>(cell->getChildren())) {
		if (node->m_item.type != m_movingItem.type) continue;

		int id = getItemId(node->m_item);

		if (id == m_movingId && id != -1) {
			node->enableMoveButton(false);
			m_movingNode = node;
			break;
		}
	}
	
	return cell;
}

void FolderPopup::updateTableView() {
	if (!m_tableView || !m_contentLayer) return;


	float scale = m_mainLayer->getScale();
	
	if (scale >= 1.f) {
		m_tableView->setContentSize(m_list->getContentSize() * scale);
		m_contentLayer->setPosition(m_contentLayer->getPosition());

		if (m_scrollbar) {
			m_scrollbar->setScaleY(1.f / scale);
			m_scrollbar->getChildByType<NineSlice>(1)->setScaleY(1.f / scale * 0.4f);
		}
	}

	if (m_mainLayer->numberOfRunningActions() != 0)
		runAction(CCSequence::create(
			CCDelayTime::create(1.f / 240.f),
			CCCallFunc::create(this, callfunc_selector(FolderPopup::updateTableView)),
			nullptr
		));
	else {
		m_tableView->setContentSize(m_list->getContentSize());
		m_contentLayer->setPosition(m_contentLayer->getPosition());

		if (m_scrollbar) {
			m_scrollbar->setScaleY(1);
			m_scrollbar->getChildByType<NineSlice>(1)->setScaleY(0.4f);
		}
	}
}

void FolderPopup::updateButtons() {
	bool canGoLeft = m_currentIndex >= 0 && !m_currentPath.empty();
	bool canGoRight = m_currentIndex != static_cast<int>(m_currentPath.size()) - 1 && !m_currentPath.empty();

	if (m_viewedLevel && !m_viewingLevel && !canGoRight)
		canGoRight = true;

	m_leftArrow->setEnabled(canGoLeft);
	m_leftArrow->setOpacity(canGoLeft ? 255 : 140);

	m_rightArrow->setEnabled(canGoRight);
	m_rightArrow->setOpacity(canGoRight ? 255 : 140);

	bool canAdd = (m_currentPath.empty() || m_currentPath.front() != 0 || m_currentPath.size() > 1 || m_currentIndex < 0) && !m_viewingLevel;
	m_addButton->setEnabled(canAdd);
	m_addButton->setOpacity(canAdd ? 255 : 140);

	m_movingLabel->setVisible(m_isMoving);
	m_moveButton->setVisible(m_isMoving);
	m_cancelMoveButton->setVisible(m_isMoving);

	if (!m_isMoving) return;

	int folder = getCurrentFolder();
	bool cantMove = m_isMoving && ((folder == m_movingId && m_movingItem.type == ItemType::Folder) || (folder == -1 && m_movingItem.type == ItemType::Level));

	for (const Item& item : m_items) {
		int id = getItemId(item);
		if (id == m_movingId && item.type == m_movingItem.type && (folder != -1 || !Utils::isSubFolder(Utils::getSaveFolder(m_searchType), id, m_searchType))) {
			cantMove = true;
			break;
		}
	}

	if (m_movingItem.type == ItemType::Folder && m_currentIndex >= 0)
		for (int i = 0; i <= m_currentIndex; i++) { 
			if (m_movingId == m_currentPath[i]) {
				cantMove = true;
				break;
			}
		}

	m_moveButton->setEnabled(!cantMove);
	m_moveButton->setOpacity(cantMove ? 140 : 255);
}

void FolderPopup::updateList() {
	if (m_list) m_list->removeFromParent();
	if (m_scrollbar) m_scrollbar->removeFromParent();

	m_list = nullptr;
	m_scrollbar = nullptr;
	m_contentLayer = nullptr;
	m_movingNode = nullptr;

	
	Mod::get()->setSavedValue("sort-inverted", m_isInverted);
	Mod::get()->setSavedValue("grid-enabled", m_isGrid);

	updateItems();
	updatePath();
	updateButtons();

	cocos2d::CCSize winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
	CCArray* cells = CCArray::create();
	std::vector<Item> cellFolders;

	for (Item& item : m_items) {
		bool isSubFolder = item.type == ItemType::Folder ? Utils::isSubFolder(Utils::getSaveFolder(m_searchType), item.id, m_searchType) : false;
		bool isSearchMismatch = Utils::toLower(item.name).find(Utils::toLower(m_search)) == std::string::npos && !m_search.empty();

		if ((isSubFolder && (m_currentIndex < 0 || m_currentPath.empty())) || isSearchMismatch)
			continue;

		if (m_isMove && item.type == ItemType::Folder && item.id == 0)
			continue;

		if (item.id == 0 && !isSearchMismatch && !m_search.empty()) continue;

		item.isMove = m_isMove;

		if (!m_isGrid) {
			cells->addObject(findMovingNode(ItemCell::create(item, static_cast<ItemCellDelegate*>(this))));
			continue;
		}

		cellFolders.push_back(item);

		if (cellFolders.size() == 3) {
			cells->addObject(findMovingNode(ItemCell::create(cellFolders, static_cast<ItemCellDelegate*>(this))));
			cellFolders.clear();
		}
	}

	if (!cellFolders.empty())
		cells->addObject(findMovingNode(ItemCell::create(cellFolders, static_cast<ItemCellDelegate*>(this))));

	if (m_isGrid && cells->count() < 3 && cells->count() > 0) {
		cells->addObject(ItemCell::create());
		cells->addObject(ItemCell::create());
	}

	m_emptyLabel->setVisible(cells->count() == 0);		
	m_emptyLabel->setString(m_search.empty() ? "Empty" : "No Results");

	ListView* listView = ListView::create(cells, m_isGrid ? 92 : 25, 289, 153);
	m_contentLayer = static_cast<CCNode*>(listView->m_tableView->getChildren()->objectAtIndex(0));
	m_tableView = listView->m_tableView;

    listView->setPrimaryCellColor(ccc3(138, 77, 46));
    listView->setSecondaryCellColor(ccc3(138, 77, 46));
    listView->setCellBorderColor(ccc4(0, 0, 0, 0));

	m_list = GJCommentListLayer::create(listView, "", ccc4(255, 255, 255, 0), 289, 153, false);
	m_list->setPosition((m_size - m_list->getContentSize()) / 2.f);
	m_mainLayer->addChild(m_list, 2);

	CCSprite* topBorder = m_list->getChildByType<CCSprite>(1);
	CCSprite* bottomBorder = m_list->getChildByType<CCSprite>(0);

	topBorder->setScale(0.85f);
	bottomBorder->setScale(0.85f);

	if (m_folderPositions.contains(getCurrentFolder()))
		m_contentLayer->setPositionY(m_folderPositions.at(getCurrentFolder()));

	if (m_contentLayer->getContentSize().height >= 175) {
		m_scrollbar = Scrollbar::create(listView->m_tableView);
        m_scrollbar->setPosition(m_list->getPosition() + ccp(m_list->getContentSize().width + 6, 0));
        m_scrollbar->setAnchorPoint({0, 0});
		m_mainLayer->addChild(m_scrollbar, 3);
	}
}

int FolderPopup::getCurrentFolder() {
	return m_currentPath.empty() || m_currentIndex < 0 ? -1 : m_currentPath[m_currentIndex];
}

int FolderPopup::getItemId(Item item) {
	return item.type == ItemType::Folder ? item.id : (m_levelIds.contains(item.level) ? m_levelIds.at(item.level) : -1);
}

void FolderPopup::onCreateFolder(CCObject*) {
	CreateFolderPopup::create(static_cast<CreateFolderDelegate*>(this))->show();
}

void FolderPopup::onSortToggle(CCObject*) {
	m_isInverted = !m_isInverted;
	
	updateList();
}

void FolderPopup::onListToggle(CCObject*) {
	if (m_listToggle->isToggled()) return m_listToggle->toggle(false);

	m_gridToggle->toggle(false);

	m_isGrid = false;
	m_folderPositions.clear();

	updateList();
}

void FolderPopup::onGridToggle(CCObject*) {
	if (m_gridToggle->isToggled()) return m_gridToggle->toggle(false);

	m_listToggle->toggle(false);

	m_isGrid = true;
	m_folderPositions.clear();

	updateList();
}

void FolderPopup::onPrevious(CCObject*) {
	if (m_viewingLevel && m_levelViewLayer) {
		m_levelViewLayer->removeFromParent();

		m_viewingLevel = nullptr;
		m_levelViewLayer = nullptr;

		return updateList();
	}

	if (m_currentIndex < 0) return;

	if (m_contentLayer && !m_items.empty())
		m_folderPositions[getCurrentFolder()] = m_contentLayer->getPositionY();

	m_currentIndex--;

	updateList();
}

void FolderPopup::onNext(CCObject*) {
	if (m_viewedLevel && !m_viewingLevel && m_currentIndex == static_cast<int>(m_currentPath.size()) - 1)
		return onLevelSee(m_viewedLevel);

	if (m_currentIndex >= static_cast<int>(m_currentPath.size()) - 1 || m_currentPath.empty()) return;

	if (m_contentLayer)
		m_folderPositions[getCurrentFolder()] = m_contentLayer->getPositionY();

	m_currentIndex++;

	updateList();
}

void FolderPopup::onMove(CCObject*) {
	if (!m_isMoving || m_movingId == 0) return;

	if (m_movingItem.type == ItemType::Level) {
		if (m_movingItem.level) {
			if (m_movingFrom == m_movingItem.level->m_levelFolder) {
				m_movingItem.level->m_levelFolder = getCurrentFolder();
			} else {
				auto obj = Mod::get()->getSavedValue<matjson::Value>("levels");
				auto id = EditorIDs::getID(m_movingItem.level);
				auto arr = obj[numToString(m_movingFrom)];
				auto newArr = matjson::Value::array();

				for (const auto& v : arr) {
					if (v.asInt().unwrapOr(id + 1) != id) {
						newArr.push(v);
					}
				}

				obj[numToString(m_movingFrom)] = newArr;

				auto arr2 = obj[numToString(getCurrentFolder())];

				if (!arr2.isArray()) {
					arr2 = matjson::Value::array();
				}

				auto didContain = false;

				for (const auto& v : arr2) {
					if (v.asInt().unwrapOr(id + 1) == id) {
						didContain = true;
						break;
					}
				}

				if (!didContain) {
					arr2.push(id);
					obj[numToString(getCurrentFolder())] = arr2;
				}

				Mod::get()->setSavedValue("levels", obj);
			}
		}
	} else
		Utils::moveFolder(m_movingItem.id, m_currentIndex, m_currentPath, m_searchType);

	m_folderPositions.erase(getCurrentFolder());
	
	onCancelMove(nullptr);
	updateList();
}

void FolderPopup::onFolderSee(int id) {
	if (m_contentLayer)
		m_folderPositions[getCurrentFolder()] = m_contentLayer->getPositionY();

	if (!m_currentPath.empty() && static_cast<int>(m_currentPath.size()) - 1 > m_currentIndex) {
		m_currentPath.erase(m_currentPath.begin() + m_currentIndex + 1, m_currentPath.end());
		m_viewedLevel = nullptr;
	}

	m_currentPath.push_back(id);
	m_currentIndex++;

	updateList();
}

void FolderPopup::onFolderRename(std::string name, int id) {
	Utils::getLevelFolders(m_searchType)->setObject(
		CCString::create(name),
		std::to_string(id)
	);

	m_folderPositions.erase(getCurrentFolder());

	float percent = abs(m_contentLayer->getPositionY()) / (m_contentLayer->getContentSize().height - 153.f);

	updateList();

	if (m_contentLayer->getContentSize().height >= 175)
		m_contentLayer->setPositionY((m_contentLayer->getContentSize().height - 153.f) * -percent);
}

void FolderPopup::onFolderCreate(std::string name) {
	int id = Utils::getNewFolderId(m_searchType);

	Utils::getLevelFolders(m_searchType)->setObject(
		CCString::create(name),
		std::to_string(id)
	);

	if (m_currentIndex >= 0)
		Utils::saveSubFolder(id, m_currentIndex, m_currentPath, matjson::Value{}, m_searchType);

	m_folderPositions.erase(getCurrentFolder());

	updateList();
}

void FolderPopup::onItemDelete(Item item) {
	if (m_isMove && item.type == ItemType::Level && m_moveLevel == item.level) {
		m_deletingItself = true;
		m_value = 0;
		return onClose(nullptr);
	}

	if (item.type == ItemType::Level && item.level) {
		auto folder = getCurrentFolder();

		if (folder != item.level->m_levelFolder) {
			auto obj = Mod::get()->getSavedValue<matjson::Value>("levels");
			auto id = EditorIDs::getID(item.level);
			auto arr = obj[numToString(folder)];
			auto newArr = matjson::Value::array();

			for (const auto& v : arr) {
				if (v.asInt().unwrapOr(id + 1) != id) {
					newArr.push(v);
				}
			}

			obj[numToString(folder)] = newArr;

			Mod::get()->setSavedValue("levels", obj);
		} else {
			item.level->m_levelFolder = 0;
		}
	}

	m_folderPositions.erase(getCurrentFolder());

	float percent = percent = abs(m_contentLayer->getPositionY()) / (m_contentLayer->getContentSize().height - 153.f);

	updateList();

	if (m_contentLayer->getContentSize().height >= 175)
		m_contentLayer->setPositionY((m_contentLayer->getContentSize().height - 153.f) * -percent);

	if (m_isMoving && item.type == m_movingItem.type) {
		int id = getItemId(item);

		if (id == m_movingId) onCancelMove(nullptr);
	}

	if (item.type == ItemType::Folder)
		for (int i = 0; i < m_currentPath.size(); i++)
			if (m_currentPath[i] == item.id) {
				m_currentPath.erase(m_currentPath.begin() + i, m_currentPath.end());
				break;
			}
}

void FolderPopup::onItemMove(CCNode* node) {
	if (m_isMoving && m_movingNode)
		m_movingNode->enableMoveButton(true);

	m_movingNode = static_cast<ItemNode*>(node);
	Item item = m_movingNode->m_item;

	m_movingId = getItemId(item);
	m_movingFrom = getCurrentFolder();

	if (m_movingId == -1) {
		m_movingNode = nullptr;
		m_movingId = 0;
		return;
	}

	m_movingNode->enableMoveButton(false);
	m_movingLabel->setString(fmt::format("Moving {} \"{}\"", item.type == ItemType::Folder ? "Folder" : "Level", item.name).c_str());
	m_movingLabel->limitLabelWidth(175.6f, 0.2, 0.00000001f);

	m_isMoving = true;
	m_movingItem = item;

	updateButtons();
}

void FolderPopup::onFolderOpen(int id) {
	m_value = id;
	onClose(nullptr);
}

void FolderPopup::onLevelSee(GJGameLevel* level) {
	m_levelViewLayer = LevelViewLayer::create(static_cast<LevelViewLayerDelegate*>(this), level);
	m_levelViewLayer->setPosition(m_list->getPosition());
	m_mainLayer->addChild(m_levelViewLayer, 1);

	if (m_list) m_list->removeFromParent();
	if (m_scrollbar) m_scrollbar->removeFromParent();

	m_emptyLabel->setVisible(false);

	m_list = nullptr;
	m_scrollbar = nullptr;
	m_contentLayer = nullptr;
	m_movingNode = nullptr;
	m_viewingLevel = level;
	m_viewedLevel = level;

	updatePath();
	updateButtons();
}

void FolderPopup::onLayerClosed() {
	onPrevious(nullptr);
}