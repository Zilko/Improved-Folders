#include "Includes.hpp"

class RenameFolderPopup : public Popup {

private:

    RenameFolderDelegate* m_delegate = nullptr;

    TextInput* m_input = nullptr;

    bool init(std::string, RenameFolderDelegate*);

    void onCreate(CCObject*);

public:

    static RenameFolderPopup* create(std::string, RenameFolderDelegate*);

};