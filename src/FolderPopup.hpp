#include "Includes.hpp"

#include "ItemCell.hpp"
#include "ItemNode.hpp"
#include "LevelViewLayer.hpp"

class FolderPopup : public Popup, public ItemCellDelegate, public CreateFolderDelegate, public TextInputDelegate, public LevelViewLayerDelegate {

private:

    SetFolderPopup* m_ogPopup = nullptr;

    CCLabelBMFont* m_movingLabel = nullptr;
    CCLabelBMFont* m_pathLabel = nullptr;
    CCLabelBMFont* m_emptyLabel = nullptr;

    CCMenuItemSpriteExtra* m_addButton = nullptr;
    CCMenuItemSpriteExtra* m_resetButton = nullptr;
    CCMenuItemSpriteExtra* m_cancelMoveButton = nullptr;
    CCMenuItemSpriteExtra* m_moveButton = nullptr;
    CCMenuItemSpriteExtra* m_searchOff = nullptr;
    CCMenuItemSpriteExtra* m_leftArrow = nullptr;
    CCMenuItemSpriteExtra* m_rightArrow = nullptr;

    CCMenuItemToggler* m_sortToggle = nullptr;
    CCMenuItemToggler* m_listToggle = nullptr;
    CCMenuItemToggler* m_gridToggle = nullptr;

    CCNode* m_contentLayer = nullptr;

    TextInput* m_searchInput = nullptr;

    Scrollbar* m_scrollbar = nullptr;

    TableView* m_tableView = nullptr;

    GJGameLevel* m_viewingLevel = nullptr;
    GJGameLevel* m_viewedLevel = nullptr;

    LevelViewLayer* m_levelViewLayer = nullptr;

    SearchType m_searchType;

    std::vector<Item> m_items;
    std::vector<int> m_currentPath;

    std::unordered_map<int, float> m_folderPositions;
    std::unordered_map<GJGameLevel*, int> m_levelIds;

    std::string m_search = "";

    bool m_isInverted = false;
    bool m_isGrid = false;

    int m_currentIndex = -1; 
    int m_value = 0;  
    
    Item m_movingItem;
    ItemType m_movingType = ItemType::Folder;
    ItemNode* m_movingNode = nullptr;
    GJGameLevel* m_moveLevel = nullptr;
    bool m_isMoving = false;
    bool m_isMove = false;
    bool m_deletingItself = false;
    bool m_resetAll = false;
    int m_movingId = 0;
    int m_movingFrom = 0;

    bool init() override;
    void textChanged(CCTextInputNode*) override;
    void keyDown(cocos2d::enumKeyCodes, double) override;
    void onClose(CCObject*) override;

    void updateTableView();
    void updateItems();
    void updateList();
    void updatePath();
    void updateButtons();

    void onSortToggle(CCObject*);
    void onListToggle(CCObject*);
    void onGridToggle(CCObject*);
    void onPrevious(CCObject*);
    void onNext(CCObject*);
    void onMove(CCObject*);
    void onCreateFolder(CCObject*);
    void onCancelSearch(CCObject*);
    void onCancelMove(CCObject*);
    void onReset(CCObject*);

    int getCurrentFolder();
    int getItemId(Item);

    ItemCell* findMovingNode(ItemCell*);

public:

    GJCommentListLayer* m_list = nullptr;
    
    static FolderPopup* create(SetFolderPopup*, bool);

    void onFolderOpen(int) override;
    void onFolderSee(int) override;
    void onLevelSee(GJGameLevel*) override;
    void onFolderCreate(std::string) override;
    void onFolderRename(std::string, int) override;
    void onItemDelete(Item) override;
    void onItemMove(CCNode*) override;
    void onLayerClosed() override;

};