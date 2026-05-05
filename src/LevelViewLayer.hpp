#include "Includes.hpp"

class NoobProgressBar : public CCNode {

private:

    CCLabelBMFont* m_percentLabel = nullptr;
    CCLabelBMFont* m_titleLabel = nullptr;

    CCSprite* m_backgroundBar = nullptr;
    CCSprite* m_progressBar = nullptr;

    CCDrawNode* m_stencil = nullptr;

    bool init(std::string);

public:

    static NoobProgressBar* create(std::string);

    void setBarColor(cocos2d::ccColor3B);

    void setBarProgress(float);

};

class LevelViewLayer : public CCLayer {

private:

    GJGameLevel* m_level = nullptr;

    LevelViewLayerDelegate* m_delegate = nullptr;

    int m_accountID = 0;

    bool init() override;

    void onProfile(CCObject*);
    void onInfo(CCObject*);
    void onBack(CCObject*);
    void onPlay(CCObject*);

public:

    static LevelViewLayer* create(LevelViewLayerDelegate*, GJGameLevel*);

};