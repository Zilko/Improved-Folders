#include "LevelViewLayer.hpp"
#include "Utils.hpp"

NoobProgressBar* NoobProgressBar::create(std::string title) {
    NoobProgressBar* ret = new NoobProgressBar();

    if (ret->init(title)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool NoobProgressBar::init(std::string title) {
    if (!CCNode::init()) return false;

    m_percentLabel = CCLabelBMFont::create("100%", "bigFont.fnt");
    m_percentLabel->setScale(0.54f);
    m_percentLabel->setPositionY(1.f);

    addChild(m_percentLabel, 2);

    m_titleLabel = CCLabelBMFont::create(title.c_str(), "bigFont.fnt");
    m_titleLabel->setScale(0.5f);
    m_titleLabel->setPositionY(20);

    addChild(m_titleLabel);

    m_backgroundBar = CCSprite::create("GJ_progressBar_001.png");
    m_backgroundBar->setColor({0, 0, 0});
    m_backgroundBar->setOpacity(125);
    m_backgroundBar->setAnchorPoint({0, 0});
    m_backgroundBar->setPosition(-m_backgroundBar->getContentSize() / 2);

    addChild(m_backgroundBar);

    m_progressBar = CCSprite::create("GJ_progressBar_001.png");
    m_progressBar->setAnchorPoint({0, 0});
    
    float width = m_progressBar->getContentSize().width;
    float height = m_progressBar->getContentSize().height;

    m_stencil = CCDrawNode::create();
    cocos2d::CCPoint vertices[] = {
        ccp(0, 0),
        ccp(width, 0),
        ccp(width, height),
        ccp(0, height)
    };
    m_stencil->drawPolygon(vertices, 4, {1, 1, 1, 1}, 0, {0, 0, 0, 0});

    CCClippingNode* clippingNode = CCClippingNode::create();
    clippingNode->setScaleX(0.992f);
    clippingNode->setScaleY(0.88f);
    clippingNode->setPosition(ccp(1.36f, 1.36f) - m_backgroundBar->getContentSize() / 2);
    clippingNode->setStencil(m_stencil);
    clippingNode->addChild(m_progressBar);

    addChild(clippingNode, 1);

    return true;
}

void NoobProgressBar::setBarColor(cocos2d::ccColor3B color) {
    m_progressBar->setColor(color);
};

void NoobProgressBar::setBarProgress(float progress) {
    m_stencil->setScaleX(progress);
    m_percentLabel->setString((std::to_string(static_cast<int>(progress * 100.f)) + "%").c_str());
}

LevelViewLayer* LevelViewLayer::create(LevelViewLayerDelegate* delegate, GJGameLevel* level) {
    LevelViewLayer* ret = new LevelViewLayer();

    ret->m_level = level;
    ret->m_delegate = delegate;

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool LevelViewLayer::init() {
    if (!CCLayer::init()) return false;

    setContentSize({289, 153});
    setAnchorPoint({0, 0});

    int difficulty;

    if (m_level->m_demon > 0)
        difficulty = m_level->m_demonDifficulty > 0 ? m_level->m_demonDifficulty + 4 : 6;
    else if (m_level->m_autoLevel)
        difficulty = -1;
    else if (m_level->m_ratings < 5)
        difficulty = 0;
    else
        difficulty = m_level->m_ratingsSum / m_level->m_ratings;

    GJDifficultySprite* diffSprite = GJDifficultySprite::create(difficulty, GJDifficultyName::Long);
    diffSprite->setScale(0.65f);
    diffSprite->setPosition({70, 109});

    if (m_level->m_levelType == GJLevelType::Editor)
        diffSprite->updateFeatureState(GJFeatureState::None);
    else
        diffSprite->updateFeatureStateFromLevel(m_level);

    addChild(diffSprite);

    CCLabelBMFont* lbl = CCLabelBMFont::create(m_level->m_levelName.c_str(), "bigFont.fnt");
    lbl->setScale(0.4f);
    lbl->setPosition({144.5f, 144});

    addChild(lbl);

    if (!m_level->isPlatformer()) {
        NoobProgressBar* bar = NoobProgressBar::create("Normal Mode");
        bar->setScale(0.5f);
        bar->setBarColor({0, 255, 0});
        bar->setBarProgress(m_level->m_newNormalPercent2.value() / 100.f);
        bar->setPosition({144.5f, 45});

        addChild(bar);

        bar = NoobProgressBar::create("Practice Mode");
        bar->setScale(0.5f);
        bar->setBarColor({0, 255, 255});
        bar->setBarProgress(m_level->m_practicePercent / 100.f);
        bar->setPosition({144.5f, 19});

        addChild(bar);
    } else {
        std::string str = m_level->m_bestTime <= 1 ? "No Best Time" : "Best Time: " + Utils::getFormattedTime(m_level->m_bestTime / 1000.f);
        lbl = CCLabelBMFont::create(str.c_str(), "goldFont.fnt");
        lbl->setPosition({144.5f, 38});
        lbl->limitLabelWidth(239, 0.425f, 0.0001f);
        
        addChild(lbl);
    }

    CCMenu* menu = CCMenu::create();
    menu->setPosition({0, 0});
    addChild(menu);

    CCSprite* spr = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
    spr->setScale(0.575f);
    CCMenuItemSpriteExtra* btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(LevelViewLayer::onPlay));
    btn->setPosition({144.5f, 92});

    menu->addChild(btn);

    std::string creator = m_level->m_creatorName;
    m_accountID = m_level->m_accountID.value() == 0;

    if (m_level->m_levelType == GJLevelType::Editor) {
        m_accountID = GJAccountManager::get()->m_accountID;
        creator = GJAccountManager::get()->m_username;
    }

    lbl = CCLabelBMFont::create(fmt::format("By {}", m_accountID == 0 ? "-" : creator).c_str(), "goldFont.fnt");
    lbl->setScale(0.4f);

    btn = CCMenuItemSpriteExtra::create(lbl, this, menu_selector(LevelViewLayer::onProfile));
    btn->setPosition({144.5f, 130});

    if (m_accountID == 0) {
        lbl->setColor({90, 255, 255});
        btn->setEnabled(false);
    }

    menu->addChild(btn);

    float yPos = diffSprite->getPositionY() - diffSprite->getContentSize().height * 0.325f - 7;
    float coinOffset = 5.f;

    if (m_level->m_stars.value() > 0) {
        lbl = CCLabelBMFont::create(std::to_string(m_level->m_stars.value()).c_str(), "bigFont.fnt");
        lbl->setPositionY(yPos);
        lbl->setScale(0.325f);

        addChild(lbl);

        spr = CCSprite::createWithSpriteFrameName("star_small01_001.png");
        spr->setPositionY(yPos);
        spr->setScale(0.65f);

        addChild(spr);

        float width = lbl->getContentSize().width * 0.325f + spr->getContentSize().width * 0.65f;
        lbl->setPositionX(70 - ((lbl->getContentSize().width * 0.325f) / 2.f) + (width / 2.f - spr->getContentSize().width * 0.65f));
        spr->setPositionX(lbl->getPositionX() + lbl->getContentSize().width * 0.175f + spr->getContentSize().width * 0.325f);
    }

    std::vector<float> coinOffsets;

    if (m_level->m_coins == 1) {
        coinOffsets = {0.f};
    } else if (m_level->m_coins == 2) {
        coinOffsets = {-coinOffset / 2.f, coinOffset / 2.f};
    } else if (m_level->m_coins == 3) {
        coinOffsets = {-coinOffset, 0.f, coinOffset};
    }

    for (int i = 0; i < m_level->m_coins; i++) {
        spr = CCSprite::createWithSpriteFrameName("usercoin_small01_001.png");
        spr->setPosition({70.f + coinOffsets[i], yPos - (m_level->m_stars.value() > 0 ? 12 : 0)});
        spr->setScale(0.65f);
        spr->setColor(m_level->m_coinsVerified ? ccc3(165, 165, 165) : ccc3(165, 113, 48));
        addChild(spr);
    }

    spr = CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png");
    spr->setScale(0.5f);
    btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(LevelViewLayer::onInfo));
    btn->setPosition({12, 12});

    menu->addChild(btn);

    spr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    spr->setScale(0.5f);
    btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(LevelViewLayer::onBack));
    btn->setPosition({17, getContentSize().height - 17});

    menu->addChild(btn);

    spr = CCSprite::createWithSpriteFrameName("GJ_downloadsIcon_001.png");
    spr->setScale(0.4f);
    spr->setPosition({203, 117});

    addChild(spr);

    lbl = CCLabelBMFont::create((m_level->m_levelType == GJLevelType::Editor ? "NA" : Utils::getFormattedAmount(m_level->m_downloads)).c_str(), "bigFont.fnt");
    lbl->limitLabelWidth(292.f, 0.25f, 0.001f);
    lbl->setAnchorPoint({0, 0.5});
    lbl->setPosition({210, 117});

    addChild(lbl);

    spr = CCSprite::createWithSpriteFrameName("GJ_likesIcon_001.png");
    spr->setScale(0.4f);
    spr->setPosition({203, 103});

    addChild(spr);

    lbl = CCLabelBMFont::create((m_level->m_levelType == GJLevelType::Editor ? "NA" : Utils::getFormattedAmount(m_level->m_likes)).c_str(), "bigFont.fnt");
    lbl->limitLabelWidth(292.f, 0.25f, 0.001f);
    lbl->setAnchorPoint({0, 0.5});
    lbl->setPosition({210, 103});

    addChild(lbl);

    spr = CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png");
    spr->setScale(0.4f);
    spr->setPosition({203, 89});

    addChild(spr);

    lbl = CCLabelBMFont::create(GJGameLevel::lengthKeyToString(m_level->m_levelLength).c_str(), "bigFont.fnt");
    lbl->limitLabelWidth(292.f, 0.25f, 0.001f);
    lbl->setAnchorPoint({0, 0.5});
    lbl->setPosition({210, 89});

    addChild(lbl);

    if (m_level->m_stars.value() > 0 && m_level->m_stars.value() < 11 && m_level->m_levelType != GJLevelType::Editor) {
        int totalOrbs = orbsForDifficulty.at(m_level->m_stars.value());
        int currentOrbs = m_level->m_orbCompletion.value() / 100.f * totalOrbs;

        spr = CCSprite::createWithSpriteFrameName("currencyOrbIcon_001.png");
        spr->setScale(0.4f);
        spr->setPosition({203, 75});

        addChild(spr);

        lbl = CCLabelBMFont::create(fmt::format("{}/{}", currentOrbs, totalOrbs).c_str(), "bigFont.fnt");
        lbl->limitLabelWidth(292.f, 0.25f, 0.001f);
        lbl->setAnchorPoint({0, 0.5});
        lbl->setPosition({210, 75});

        addChild(lbl);
    }

    return true;
}   

void LevelViewLayer::onProfile(CCObject*) {
    ProfilePage::create(m_accountID, false)->show();
}

void LevelViewLayer::onInfo(CCObject*) {
    std::string str;

    str += fmt::format(" <cy>{}</c>\n ", m_level->m_levelName);
    str += fmt::format(" <cg>Total Attempts</c>: {}\n ", m_level->m_attempts.value());
    str += fmt::format(" <cl>Total Jumps</c>: {}\n ", m_level->m_jumps.value());

    if (m_level->m_levelType == GJLevelType::Editor) {
        str += fmt::format(" <cp>Objects</c>: {} ", m_level->m_objectCount.value());
    } else {
        str += fmt::format(" <cp>Normal</c>: {}%\n ", m_level->m_newNormalPercent2.value());
        str += fmt::format(" <co>Practice</c>: {}% ", m_level->m_practicePercent);
    }

    FLAlertLayer::create("Level Stats", str.c_str(), "Ok")->show();
}

void LevelViewLayer::onBack(CCObject*) {
    m_delegate->onLayerClosed();
}

void LevelViewLayer::onPlay(CCObject*) {
    GameLevelManager::get()->gotoLevelPage(m_level);
}