#include "Utils.hpp"
#include "RenameFolderPopup.hpp"
#include "FolderPopup.hpp"

ButtonMenu* ButtonMenu::create(ItemCellDelegate* delegate) {
    ButtonMenu* ret = new ButtonMenu();

    ret->m_delegate = delegate;

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

ItemNode* ItemNode::create(Item item, ItemCellDelegate* delegate, bool isGrid) {
    ItemNode* ret = new ItemNode();

    ret->m_delegate = delegate;
    ret->m_isGrid = isGrid;
    ret->m_item = item;
    ret->m_isLevel = item.type == ItemType::Level;
    
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    
    delete ret;
    return nullptr;
}

bool ButtonMenu::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    cocos2d::CCPoint pos = static_cast<FolderPopup*>(m_delegate)->m_list->convertToNodeSpace(touch->getLocation());

    if (pos.x < 0 || pos.y < 0 || pos.x > 289 || pos.y > 153) return false;

    return CCMenu::ccTouchBegan(touch, event);
}

bool ItemNode::init() {
    CCScale9Sprite* bg = CCScale9Sprite::create("square02b_001.png");
	bg->setColor({0, 0, 0});
    bg->setOpacity(m_item.id == 0 && !m_isLevel ? 25 : 20);
    bg->setPosition({5, 0});
    bg->setAnchorPoint({0, 0});
    bg->setContentSize(ccp(m_isGrid ? 90.f : 279, m_isGrid ? 87 : 22) * 4);
    bg->setScale(0.25f);

	addChild(bg, 1);

    ButtonMenu* menu = ButtonMenu::create(m_delegate);
    menu->setPosition({0, 0});

    addChild(menu);

    CCSprite* spr = CCSprite::createWithSpriteFrameName(m_isLevel ? "geode.loader/file-add.png" : "folderIcon_001.png");

    //  ignore
    if (m_isLevel) {
        spr = CCSprite::create("vro.png"_spr);
    }

    spr->setScale((m_isGrid ? 0.7f : 0.5f) - (m_isLevel ? (m_isGrid ? 1.17f : 0.9f) : 0.f));

    //  ignore
    if (m_isLevel) {
        spr->setScale(spr->getScale() * (m_isGrid ? -1.49f : -1.1f));
    }

    CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ItemNode::onSee));
    btn->setPosition(m_isGrid ? ccp(50, m_item.isMove && m_isLevel ? 54 : 64) : ccp(20, 11));

    menu->addChild(btn);

    ButtonSprite* btnSpr = ButtonSprite::create(m_item.isMove ? "Move" : (m_isLevel ? "View" : "Open"), "bigFont.fnt", "GJ_button_01.png");
    btnSpr->setScale(0.425f);
    btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(ItemNode::onOpen));
    btn->setPosition(m_isGrid ? ccp(50, 17) : ccp(256, 11));

    menu->addChild(btn);
    
    if (m_item.isMove && m_isLevel) btn->setVisible(false);

    m_moveSprite1 = CCSprite::createWithSpriteFrameName("folderIcon_001.png");
    m_moveSprite1->setOpacity(170);
    m_moveSprite1->setScale(m_isGrid ? 0.33f : 0.375f);

    m_moveSprite2 = CCSprite::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
    m_moveSprite2->setScale(0.85f);
    m_moveSprite2->setFlipX(true);
    m_moveSprite2->setOpacity(190);
    m_moveSprite2->setPosition(m_moveSprite1->getContentSize() / 2 + ccp(7, 9.5f));

    m_moveSprite1->addChild(m_moveSprite2);

    m_moveButton = CCMenuItemSpriteExtra::create(m_moveSprite1, this, menu_selector(ItemNode::onMove));
    m_moveButton->setCascadeOpacityEnabled(true);
    m_moveButton->setPosition(m_isGrid ? ccp(15, 60) : ccp(197, 11));

    menu->addChild(m_moveButton);

    if (m_item.id == 0 && !m_isLevel) {
        m_moveButton->setEnabled(false);
        m_moveButton->setOpacity(30);
        m_moveSprite2->setOpacity(30);
    }

    if (m_item.isMove || (!m_item.isMove && !Mod::get()->getSettingValue<bool>("enable-moving-folders")) && !m_isLevel) m_moveButton->setVisible(false);

    spr = CCSprite::createWithSpriteFrameName("GJ_resetBtn_001.png");
    spr->setScale(m_isGrid ? 0.575f : 0.7f);
    btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ItemNode::onDelete));
    btn->setPosition(m_isGrid ? ccp(15, 77) : ccp(219, 11));

    menu->addChild(btn);

    if (m_item.isMove && !m_isGrid && m_isLevel) btn->setPositionX(260);

    if ((m_item.id == 0 && !m_isLevel) || (m_isLevel && m_item.level->m_levelFolder == 0)) {
        btn->setCascadeOpacityEnabled(true);
        btn->setEnabled(false);
        btn->setOpacity(30);
    }

    float trashStart = btn->getPosition().x - btn->getContentSize().width / 2;

    CCLabelBMFont* lbl = CCLabelBMFont::create(m_item.name.c_str(), "bigFont.fnt");
    lbl->limitLabelWidth(75, 0.4f, 0.001f);
    btn = CCMenuItemSpriteExtra::create(lbl, this, menu_selector(ItemNode::onRename));
    btn->setPosition(m_isGrid ? ccp(50, m_item.isMove && m_isLevel ? 27 : 41) : ccp(btn->getContentSize().width / 2 + 33, 11));
    btn->setEnabled(!m_isLevel);

    menu->addChild(btn);
    
    int folderCount = Utils::getFolderCount(Utils::getSaveFolder(m_item.searchType), m_item.id);
    int levelCount = Utils::getLevelCount(m_item.id, m_item.searchType);

    if (folderCount < 0) folderCount = 0;
    
    bool empty = levelCount == 0 && folderCount == 0;

    if (!m_isLevel) {
        float middleSpace = trashStart - btn->getContentSize().width / 2 + btn->getPosition().x;

        std::string str = empty ? "Empty" : fmt::format("{} Level{}", levelCount, levelCount == 1 ? "" : "s");

        lbl = CCLabelBMFont::create(str.c_str(), "chatFont.fnt");
        lbl->setColor({0, 0, 0});
        lbl->setOpacity(72);
        lbl->setPosition(m_isGrid ? ccp(93, 82) : (empty || m_item.id == 0 ? ccp(158, 11) : ccp(158, 15)));
        lbl->limitLabelWidth(m_isGrid ? 65.f : middleSpace - 30.f, 0.39f, 0.0001f);
        lbl->setAnchorPoint({m_isGrid ? 1.f : 0.5f, 0.5f});

        addChild(lbl);

        if (!empty && m_item.id != 0) {
            str = fmt::format("{} Folder{}", folderCount, folderCount == 1 ? "" : "s");

            lbl = CCLabelBMFont::create(str.c_str(), "chatFont.fnt");
            lbl->setColor({0, 0, 0});
            lbl->setOpacity(72);
            lbl->setPosition(m_isGrid ? ccp(93, 74) : ccp(158, 7));
            lbl->limitLabelWidth(m_isGrid ? 65.f : middleSpace - 30.f, 0.39f, 0.0001f);
            lbl->setAnchorPoint({m_isGrid ? 1.f : 0.5f, 0.5f});

            addChild(lbl);
        }
    }

    return true;
}

void ItemNode::enableMoveButton(bool enabled) {
    m_moveButton->setEnabled(enabled);
    m_moveButton->setOpacity(enabled ? 255 : 30);

    m_moveSprite1->setOpacity(enabled ? 170 : 30);
    m_moveSprite2->setOpacity(enabled ? 210 : 30);
}

void ItemNode::onOpen(CCObject*) {
    if (!m_isLevel) return m_delegate->onFolderOpen(m_item.id);

    GameLevelManager::get()->gotoLevelPage(m_item.level);
}

void ItemNode::onSee(CCObject*) {
    m_isLevel ? m_delegate->onLevelSee(m_item.level) : m_delegate->onFolderSee(m_item.id);
}

void ItemNode::onMove(CCObject*) {
    m_delegate->onItemMove(static_cast<CCNode*>(this));
}

void ItemNode::onRename(CCObject*) {
    RenameFolderPopup::create(m_item.name, static_cast<RenameFolderDelegate*>(this))->show();
}

void ItemNode::onFolderRename(std::string name) {
    m_delegate->onFolderRename(name, m_item.id);
}

void ItemNode::onDelete(CCObject*) {
    if (!m_isLevel && m_item.id == 0) return;
    
    geode::createQuickPopup(
        "Warning",
        fmt::format("Are you sure you want to <cr>remove</c> this {}?\n<cl>({})</c>",
            m_isLevel ? "level" : "folder",
            m_isLevel ? "The level itself won't be deleted." : "The levels in it won't be deleted."
        ),
        "Cancel", "Yes",
        [this](auto, bool yes) {
            if (!yes) return;
            
            if (!m_isLevel)
                Utils::removeFolder(m_item.id, m_item.searchType);

            m_delegate->onItemDelete(m_item);
        }
    );
}