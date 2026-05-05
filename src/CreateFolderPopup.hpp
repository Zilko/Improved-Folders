#include "Includes.hpp"

class CreateFolderPopup : public Popup {

private:

    CreateFolderDelegate* m_delegate = nullptr;

    TextInput* m_input = nullptr;

    bool init() override;

    void onCreate(CCObject*);

public:

    static CreateFolderPopup* create(CreateFolderDelegate*);

};