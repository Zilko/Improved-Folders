#include "CreateFolderPopup.hpp"

CreateFolderPopup* CreateFolderPopup::create(CreateFolderDelegate* delegate) {
    CreateFolderPopup* ret = new CreateFolderPopup();

    ret->m_delegate = delegate;

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool CreateFolderPopup::init() {
    Popup::init(300, 135);

    setTitle("Create Folder");

    m_input = TextInput::create(170, "Name", "bigFont.fnt");
    m_input->getInputNode()->setMaxLabelLength(60);
    m_input->setPosition({m_size.width / 2, 74});

    m_mainLayer->addChild(m_input);

    ButtonSprite* btnSpr = ButtonSprite::create("Create");
    btnSpr->setScale(0.8f);

    CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(CreateFolderPopup::onCreate));
    btn->setPosition(m_size / 2 - ccp(0, 40));

    m_buttonMenu->addChild(btn);

    return true;
}

void CreateFolderPopup::onCreate(CCObject*) {
    std::string name = m_input->getString();

    if (name.empty())
        return FLAlertLayer::create("Error", "Folder name can't be <cl>empty</c>.", "Ok")->show();
    
    onClose(nullptr);

    m_delegate->onFolderCreate(name);
}