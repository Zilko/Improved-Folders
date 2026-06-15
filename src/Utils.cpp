#include "Utils.hpp"

std::string Utils::toLower(std::string str) {
	for (char& c : str) c = std::tolower(c);
	return str;
}

std::string Utils::getFormattedTime(float time) {
    std::string ret = "";
    int hours = static_cast<int>(time / 3600);
    time = std::fmod(time, 3600);
    int minutes = static_cast<int>(time / 60);
    time = std::fmod(time, 60);
    int seconds = static_cast<int>(time);
    int milliseconds = static_cast<int>((time - seconds) * 100);

    if (hours > 0)
        ret = fmt::format("{:02}:{:02}:{:02}.{:02}", hours, minutes, seconds, milliseconds);
    else if (minutes > 0)
        ret = fmt::format("{:02}:{:02}.{:02}", minutes, seconds, milliseconds);
    else
        ret = fmt::format("{}.{:03}s", seconds, static_cast<int>((time - seconds) * 1000));

    return ret;
}

std::string Utils::getFormattedAmount(int amount) {
	std::string str = std::to_string(amount);
	std::reverse(str.begin(), str.end());

	std::string ret = "";

	for (int i = 0; i < str.size(); i++) {
		ret += str[i];
		if ((i + 1) % 3 == 0 && i != 0 && i != str.size() - 1) ret += ",";
	}

	std::reverse(ret.begin(), ret.end());

	return ret;
}

CCSprite* Utils::createTogglerSprite(const char* iconName, bool selected, bool isSpriteFrameName) {
	CCSprite* spr = CCSprite::create(selected ? "GJ_button_02.png" : "GJ_button_01.png");
	CCSprite* icon = isSpriteFrameName ? CCSprite::createWithSpriteFrameName(iconName) : CCSprite::create(iconName);
	icon->setPosition(spr->getContentSize() / 2);
	spr->addChild(icon);

	return spr;
}

CCArrayExt<GJGameLevel*> Utils::getLevels(int folder, SearchType type) {
	auto ret = CCArrayExt<GJGameLevel*>(type == SearchType::MyLevels ? LocalLevelManager::get()->getCreatedLevels(folder) : GameLevelManager::get()->getSavedLevels(false, folder));
	auto obj = Mod::get()->getSavedValue<matjson::Value>("levels");

	if (obj.contains(numToString(folder))) {
		for (const auto& v : obj[numToString(folder)]) {
			int id = v.asInt().unwrapOr(0);
			
			if (id <= 0) {
				continue;
			}

			if (auto level = GameLevelManager::get()->getSavedLevel(id)) {
				ret.push_back(level);
			} else {
				for (auto level : CCArrayExt<GJGameLevel*>(LocalLevelManager::get()->m_localLevels)) {
					if (EditorIDs::getID(level) == id) {
						ret.push_back(level);
						break;
					}
				}
			}
		}
	}

	return ret;
}

CCArrayExt<GJGameLevel*> Utils::getAllLevels(SearchType type) {
	return CCArrayExt<GJGameLevel*>(type == SearchType::MyLevels ? LocalLevelManager::get()->m_localLevels : GameLevelManager::get()->getSavedLevels(false, 0));
}

CCDictionary* Utils::getLevelFolders(SearchType type) {
	return type == SearchType::MyLevels ? GameLevelManager::get()->m_localLevelsFolders : GameLevelManager::get()->m_onlineFolders;
}

void Utils::removeMissingFolders(matjson::Value* folder, SearchType type) {
	for (matjson::Value& subFolder : *folder) {
		std::string idStr = subFolder.getKey().value_or("0");
        int id = numFromString<int>(idStr).unwrapOr(0);

		if (id < 1) continue;

		if (folderExists(id, type))
			removeMissingFolders(&subFolder, type);
		else
			folder->erase(idStr);
	}
}

void Utils::removeFolder(int folderId, SearchType type) {
	for (GJGameLevel* level : getLevels(folderId, type))
		level->m_levelFolder = 0;

	getLevelFolders(type)->removeObjectForKey(std::to_string(folderId));

	std::function<bool(matjson::Value*)> fun;

	fun = [&fun, &folderId, type](matjson::Value* folder) -> bool {
		for (matjson::Value& subFolder : *folder) {
			std::string idStr = subFolder.getKey().value_or("0");
			int id = numFromString<int>(idStr).unwrapOr(0);

			if (id == folderId) {
				removeSubFolders(subFolder, type);
				folder->erase(idStr);
				return true;
			}
			else if (fun(&subFolder)) return true; 
		}	

		return false;
	};
	
	matjson::Value saveFolder = Utils::getSaveFolder(type);
    matjson::Value* folder = &saveFolder;
    
    fun(folder);

    Mod::get()->setSavedValue(getSaveFolderId(type), *folder);
}

void Utils::removeSubFolders(const matjson::Value& folder, SearchType type) {
	for (const matjson::Value& subFolder : folder) {
		std::string idStr = subFolder.getKey().value_or("0");
			int id = numFromString<int>(idStr).unwrapOr(0);

		if (id == 0) continue;

		for (GJGameLevel* level : getLevels(id, type))
			level->m_levelFolder = 0;

		getLevelFolders(type)->removeObjectForKey(idStr);

		removeSubFolders(subFolder, type);
	}
}

int Utils::getLevelCount(int folder, SearchType type) {
	return getLevels(folder, type).size();
}

int Utils::getFolderCount(const matjson::Value& folder, int id) {
	if (id == 0) return -1;

	for (const matjson::Value& subFolder : folder) {
		if (id == numFromString<int>(subFolder.getKey().value_or("0")).unwrapOr(0))
			return subFolder.size();

		int count = getFolderCount(subFolder, id);
		if (count != -1) return count;
	}

	return -1;
}

std::vector<int> Utils::getFolderPath(int folderId, bool previous, SearchType type) {
	if (folderId == 0) return {};

	matjson::Value folder = getSaveFolder(type);
	std::vector<int> ret;
	std::function<bool(const matjson::Value&)> fun;

	fun = [&fun, &ret, folderId](const matjson::Value& folder) -> bool {
		for (const matjson::Value& subFolder : folder) {
			int id = numFromString<int>(subFolder.getKey().value_or("0")).unwrapOr(0);

			ret.push_back(id);

			if (id == folderId) return true;
			if (fun(subFolder)) return true; 
			
			ret.pop_back();
		}

		return false;
	};

	fun(folder);

	if (!ret.empty() && previous) ret.pop_back();

	return ret;
}

bool Utils::folderExists(int id, SearchType type) {
	for (const auto& [idStr, name] : CCDictionaryExt<std::string, CCString*>(getLevelFolders(type)))
		if (id == numFromString<int>(idStr).unwrapOr(0)) return true;

	for (GJGameLevel* level : getAllLevels(type))
		if (level->m_levelFolder == id) return true;

	return false;
}

bool Utils::isSubFolder(matjson::Value folder, int id, SearchType type) {
	bool isMainFolder = folder.getKey().value_or(getSaveFolderId(type)) != getSaveFolderId(type);

	for (matjson::Value subFolder : folder) {
		if (id == numFromString<int>(subFolder.getKey().value_or("0")).unwrapOr(0) && isMainFolder)
			return true;

		if (isSubFolder(subFolder, id, type)) return true;
	}

	return false;
}

std::string Utils::getFolderName(int id, SearchType type) {
	if (id < 1) return "All Levels";

	std::string name = GameLevelManager::get()->getFolderName(id, type == SearchType::MyLevels);

	if (name.empty()) {
		name = "Unnamed Folder";

		getLevelFolders(type)->setObject(
			CCString::create(name),
			std::to_string(id)
		);
	}

	return name;
}

std::vector<Item> Utils::getAllFolders(bool isInverted, SearchType type) {
	std::vector<Item> items = isInverted ? std::vector<Item>{} : std::vector<Item>{ {ItemType::Folder, "All Levels", 0} };

	std::unordered_set<int> foundFolders;
	foundFolders.insert(0);

	for (GJGameLevel* level : getAllLevels(type)) {
		int id = level->m_levelFolder;

		if (foundFolders.contains(id) || id == 0) continue;

		foundFolders.insert(id);		
		items.push_back({
			.type = ItemType::Folder,
			.name = getFolderName(id, type),
			.id = id,
			.searchType = type
		});
	}

	CCDictionaryExt<std::string, CCString*> folders = getLevelFolders(type);
	
	for (const auto& [idStr, name] : folders) {
		int id = numFromString<int>(idStr).unwrapOr(0);

		if (foundFolders.contains(id)) continue;

		foundFolders.insert(id);
		items.push_back({
			.type = ItemType::Folder,
			.name = name->m_sString,
			.id = id,
			.searchType = type
		});
	}

	if (isInverted) {
		std::vector<Item> realItems = { { .name = "All Levels" } };

		for (int i = items.size() - 1; i >= 0; i--)
			realItems.push_back(items[i]);

		items = realItems;
	}

	return items;
}

std::vector<Item> Utils::getItemsForPath(const std::vector<int>& path, int index, bool isInverted, SearchType type) {
	std::vector<Item> folders;
	std::vector<Item> levels;
	std::vector<Item> items;

	if (path.size() == 1 && path.front() == 0) {
		
		for (GJGameLevel* level : getAllLevels(type))
			levels.push_back({
				.type = ItemType::Level,
				.name = level->m_levelName,
				.level = level,
				.searchType = type
			});

	} else {
		if (index >= static_cast<int>(path.size())) return items;

		matjson::Value folder = getSaveFolder(type);
		bool found = false;

		for (int i = 0; i <= index; i++) {
			std::string key = std::to_string(path[i]);

			if (folder.contains(key)) {
				folder = folder[key];
				if (i == index) found = true;
			}
		}

		if (found)
			for (const matjson::Value& obj : folder) {
				int id = numFromString<int>(obj.getKey().value_or("0")).unwrapOr(0);
				if (id < 1) continue;

				folders.push_back({
					.type = ItemType::Folder,
					.name = getFolderName(id, type),
					.id = id,
					.searchType = type
				});
			}
	}

	if (path[index] != 0)
		for (GJGameLevel* level : getLevels(path[index], type))
			levels.push_back({
				.type = ItemType::Level,
				.name = level->m_levelName,
				.level = level,
				.searchType = type
			});

	if (isInverted) {
		std::reverse(folders.begin(), folders.end());
		std::reverse(levels.begin(), levels.end());
	}

	for (Item item : folders) items.push_back(item);
	for (Item item : levels) items.push_back(item);

	return items;
}

int Utils::getNewFolderId(SearchType type) {
	std::unordered_set<int> foundFolders;
	foundFolders.insert(0);

	for (GJGameLevel* level : getAllLevels(type)) {
		int id = level->m_levelFolder;
		if (id == 0) continue;
		
		if (foundFolders.contains(id)) continue;
		foundFolders.insert(id);
	}

	for (CCString* str : CCArrayExt<CCString*>(getLevelFolders(type)->allKeys())) {
		int id = numFromString<int>(str->m_sString).unwrapOr(0);
		
		if (foundFolders.contains(id)) continue;
		foundFolders.insert(id);
	}

	int id = 0;
	while (foundFolders.contains(id)) id++;

	return id;
}

std::string Utils::getSaveFolderId(SearchType type) {
	return type == SearchType::MyLevels ? "local-folders" : "online-folders";
}

matjson::Value Utils::getSaveFolder(SearchType type) {
	std::string id = getSaveFolderId(type);

    if (!Mod::get()->hasSavedValue(id))
        Mod::get()->setSavedValue(id, matjson::Value{});

    return Mod::get()->getSavedValue<matjson::Value>(id);
}

void Utils::saveSubFolder(int folderId, int index, const std::vector<int>& path, matjson::Value value, SearchType type) {
    matjson::Value folder = getSaveFolder(type);
    matjson::Value* tempFolder = &folder;

    for (int i = 0; i <= index; i++) {
        std::string key = std::to_string(path[i]);

        if (!tempFolder->contains(key))
            (*tempFolder)[key] = matjson::Value{};

        tempFolder = &(*tempFolder)[key];
    }

    (*tempFolder)[std::to_string(folderId)] = value;

    Mod::get()->setSavedValue(getSaveFolderId(type), folder);
}

void Utils::moveFolder(int folderId, int index, const std::vector<int>& path, SearchType type) {
	matjson::Value saveFolder = Utils::getSaveFolder(type);
	matjson::Value foundFolder = matjson::Value{};

	if (saveFolder.contains(std::to_string(folderId)) || isSubFolder(saveFolder, folderId, type)) {
		std::function<bool(matjson::Value*)> fun;

		fun = [&fun, &foundFolder, &folderId](matjson::Value* folder) -> bool {
			for (matjson::Value& subFolder : *folder) {
				std::string idStr = subFolder.getKey().value_or("0");
				int id = numFromString<int>(idStr).unwrapOr(0);

				if (id == folderId) {
					foundFolder = subFolder;
					folder->erase(idStr);
					return true;
				}
				else if (fun(&subFolder)) return true; 
			}	

			return false;
		};
		
		matjson::Value* folder = &saveFolder;

		fun(folder);
		Mod::get()->setSavedValue(getSaveFolderId(type), *folder);
	}

	saveSubFolder(folderId, index, path, foundFolder, type);
}