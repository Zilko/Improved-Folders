#pragma once

using namespace geode::prelude;

#include <cvolton.level-id-api/include/EditorIDs.hpp>

const std::unordered_map<int, int> orbsForDifficulty = {
    {1, 0},
    {2, 50},
    {3, 75},
    {4, 125},
    {5, 175},
    {6, 225},
    {7, 275},
    {8, 350},
    {9, 425},
    {10, 500}
};

enum ItemType {
    Folder = 1,
    Level = 2
};

struct Item {
    ItemType type = ItemType::Folder;
    std::string name = "";
    int id = 0;
    GJGameLevel* level = nullptr;
    SearchType searchType = SearchType::MyLevels;
    bool isMove = false;
};

class ItemCellDelegate {

public:

    virtual void onFolderSee(int) {}

    virtual void onLevelSee(GJGameLevel*) {}

    virtual void onFolderOpen(int) {}

    virtual void onItemMove(CCNode*) {}

    virtual void onItemDelete(Item) {}

    virtual void onFolderRename(std::string, int) {}

};

class CreateFolderDelegate {

public:

    virtual void onFolderCreate(std::string) {}

};

class RenameFolderDelegate {

public:

    virtual void onFolderRename(std::string) {}

};

class LevelViewLayerDelegate {

public:

    virtual void onLayerClosed() {}

};