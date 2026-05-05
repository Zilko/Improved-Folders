#include "RenameFolderPopup.hpp"

RenameFolderPopup* RenameFolderPopup::create(std::string name, RenameFolderDelegate* delegate) {
    RenameFolderPopup* ret = new RenameFolderPopup();

    if (ret->init(name, delegate)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool RenameFolderPopup::init(std::string name, RenameFolderDelegate* delegate) {
    Popup::init(300, 135);
    
    setTitle("Rename Folder");
    
    m_delegate = delegate;

    m_input = TextInput::create(170, "Name", "bigFont.fnt");
    m_input->getInputNode()->setMaxLabelLength(60);
    m_input->setPosition({m_size.width / 2, 74});
    m_input->setString(name);

    m_mainLayer->addChild(m_input);

    ButtonSprite* btnSpr = ButtonSprite::create("Rename");
    btnSpr->setScale(0.8f);

    CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(btnSpr, this, menu_selector(RenameFolderPopup::onCreate));
    btn->setPosition(m_size / 2 - ccp(0, 40));

    m_buttonMenu->addChild(btn);

    return true;
}

void RenameFolderPopup::onCreate(CCObject*) {
    std::string name = m_input->getString();

    if (name.empty())
        return FLAlertLayer::create("Error", "Folder name can't be <cl>empty</c>.", "Ok")->show();
    
    onClose(nullptr);

    m_delegate->onFolderRename(name);
}