#include "MPRank.hpp"
#include "utils.hpp"
#include "Variables.hpp"

bool MPRankItem::init(int rank, std::string name, int count, bool scaleDown, bool countFeatured) {
    if (!CCNode::init()) return false;
    this->setID("MPRankItem");

    bool geodeTheme = Loader::get()->getLoadedMod("geode.loader")->getSettingValue<bool>("enable-geode-theme");
    this->setContentSize({ 330.f , (scaleDown ? 20.f : 40.f) });

    auto menuBG = CCScale9Sprite::create("square02b_001.png");
    menuBG->setScale(0.3f);
    menuBG->setContentSize({ 330.f / 0.3f , (scaleDown ? 20.f : 40.f) / 0.3f });
    menuBG->setColor(geodeTheme ? ccColor3B { 255 , 255 , 255 } : ccColor3B { 0 , 0 , 0 });
    menuBG->setOpacity(geodeTheme ? 15 : 50);
    menuBG->setPosition({ this->getContentWidth() / 2 , this->getContentHeight() / 2});
    menuBG->setID("background");
    this->addChild(menuBG);

    auto label = CCLabelBMFont::create((std::to_string(rank) + std::string(". ") + name).c_str(), "goldFont.fnt");
    label->setScale(scaleDown ? 0.4f : 0.8f);
    label->setAnchorPoint({ 0 , 0.5 });
    label->setPosition({ 5.f, this->getContentHeight() / 2 });
    label->setID("label-developer");
    this->addChild(label);

    auto geodeSprite = CCSprite::createWithSpriteFrameName(
        countFeatured ? "geode.loader/geode-logo-outline-gold.png" : "geode.loader/geode-logo-outline.png"
    );
    geodeSprite->setScale(scaleDown ? 0.4f : 0.8f);
    geodeSprite->setAnchorPoint({ 1, 0.5 });
    geodeSprite->setPosition({ this->getContentWidth() - 5.f , this->getContentHeight() / 2 });
    geodeSprite->setID("geode-sprite");
    this->addChild(geodeSprite);

    auto countLabel = CCLabelBMFont::create(std::to_string(count).c_str(), "bigFont.fnt");
    countLabel->setScale(scaleDown ? 0.4f : 0.8f);
    countLabel->setAnchorPoint({ 1 , 0.5 });
    countLabel->setPosition({ this->getContentWidth() - geodeSprite->getScaledContentWidth() - 10.f , this->getContentHeight() / 2 });
    countLabel->setID("label-count");
    this->addChild(countLabel);

    return true;
}

MPRankItem* MPRankItem::create(int rank, std::string name, int count, bool scaleDown, bool countFeatured) {
    auto ret = new MPRankItem();
    if (ret->init(rank, name, count, scaleDown, countFeatured)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool MPRankPopup::init(float width, float height) {
    // straight up copied from GeodePopup definition
    bool geodeTheme = Loader::get()->getLoadedMod("geode.loader")->getSettingValue<bool>("enable-geode-theme");
    const char* bg = geodeTheme ? "geode.loader/GE_square01.png" : "GJ_square01.png";

    if (!Popup<>::initAnchored(width, height, bg))
        return false;
    this->setID("MPRankPopup");

    this->setCloseButtonSpr(
        CircleButtonSprite::createWithSpriteFrameName(
            "geode.loader/close.png", .85f,
            (geodeTheme ? CircleBaseColor::DarkPurple : CircleBaseColor::Green)
        )
    );

    return true;
}

void MPRankPopup::loadLeaderboard() {
    int c = 1;
    sort(DevsData::gDevelopers.as_object().begin(), DevsData::gDevelopers.as_object().end(), compareCount);

    for (auto& [key, value] : DevsData::gDevelopers.as_object()) {
        int count = value["count"].as_int();
        if (Mod::get()->getSavedValue<bool>("count-featured", false)) {
            count += value["featured"].as_int();
        }

        auto mpItem = MPRankItem::create(
            c++,
            key,
            count,
            Mod::get()->getSavedValue<bool>("scale-down", false),
            Mod::get()->getSavedValue<bool>("count-featured", false)
        );
        m_scrollLayer->m_contentLayer->addChild(mpItem);
    }

    m_scrollLayer->m_contentLayer->updateLayout();
    m_scrollLayer->scrollToTop();
}

void MPRankPopup::onScaleDown(CCObject* sender) {
    Mod::get()->setSavedValue<bool>("scale-down", !as<CCMenuItemToggler*>(sender)->m_toggled);
    m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);

    MPRankPopup::loadLeaderboard();
}

void MPRankPopup::onCountFeatured(CCObject* sender) {
    Mod::get()->setSavedValue<bool>("count-featured", !as<CCMenuItemToggler*>(sender)->m_toggled);
    m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);

    MPRankPopup::loadLeaderboard();
}

void MPRankPopup::onRetryLoading(CCObject* sender) {
    m_loadingCircle->setVisible(true);
    m_failMenu->setVisible(false);

    int page = 1;

    m_listener.bind([this, page] (web::WebTask::Event* e) mutable {
        if (web::WebResponse* res = e->getValue()) {
            if (res->ok()) {
                auto data = res->json().unwrapOr("Uh oh!")["payload"]["data"];

                for (auto& mod : data.as_array()) {
                    matjson::Array devs = mod["developers"].as_array();
                    for (auto& dev : devs) {
                        if (dev["is_owner"].as_bool()) {
                            auto displayName = dev["display_name"].as_string();

                            if (Loader::get()->isModInstalled(mod["id"].as_string())) {
                                auto leMod = Loader::get()->getInstalledMod(mod["id"].as_string());
                                if (leMod->getDeveloper() != displayName) {
                                    if (!DevsData::gAliases.contains(leMod->getDeveloper())) {
                                        DevsData::gAliases[leMod->getDeveloper()] = displayName;
                                    } else if (DevsData::gAliases[leMod->getDeveloper()] != displayName) {
                                        log::warn("bro what? {}", leMod->getDeveloper());
                                    }
                                }
                            }

                            if (!DevsData::gDevelopers.contains(displayName)) {
                                DevsData::gDevelopers[displayName] = matjson::Object();
                                DevsData::gDevelopers[displayName]["username"] = dev["username"].as_string();
                                DevsData::gDevelopers[displayName]["count"] = 0;
                                DevsData::gDevelopers[displayName]["featured"] = 0;
                            }
                            
                            // lmao this is disgusting
                            DevsData::gDevelopers[displayName]["count"] = DevsData::gDevelopers[displayName]["count"].as_int() + 1;
                            DevsData::gDevelopers[displayName]["featured"] = DevsData::gDevelopers[displayName]["featured"].as_int() + (mod["featured"].as_bool() ? 1 : 0);

                            break;
                        }
                    }
                }

                if (data.as_array().size() == 100) {
                    web::WebRequest req = web::WebRequest();

                    req.param("gd", "2.206");
                    req.param("status", "accepted");
                    req.param("per_page", "100");
                    req.header("Content-Type", "application/json");
                    std::chrono::seconds timeout(30);
                    req.timeout(timeout);

                    req.param("page", std::to_string(++page));

                    m_listener.setFilter(req.get("https://api.geode-sdk.org/v1/mods"));
                } else {
                    DevsDataState::isFinished = true;
                    DevsDataState::isInProgress = false;

                    if (auto popup = getChildOfType<MPRankPopup>(CCDirector::get()->getRunningScene(), 0)) {
                        m_loadingCircle->setVisible(false);
                        MPRankPopup::loadLeaderboard();
                    }
                }
            } else {
                log::error("Your internet connection is silly!");
                DevsDataState::isInternetSilly = true;
                DevsDataState::isInProgress = false;

                if (auto popup = getChildOfType<MPRankPopup>(CCDirector::get()->getRunningScene(), 0)) {
                    m_loadingCircle->setVisible(false);
                    m_failMenu->setVisible(true);
                }
            }

        } else if (web::WebProgress* p = e->getProgress()) {
            // log::info("Progress: {}", p->downloadProgress().value_or(0.f));
            DevsDataState::isInProgress = true;
        } else if (e->isCancelled()) {
            log::warn("The request was cancelled... So sad :(");
            DevsDataState::isInternetSilly = true;
            DevsDataState::isInProgress = false;

            if (auto popup = getChildOfType<MPRankPopup>(CCDirector::get()->getRunningScene(), 0)) {
                m_loadingCircle->setVisible(false);
                m_failMenu->setVisible(true);
            }
        }
    });

    web::WebRequest req = web::WebRequest();

    req.param("gd", "2.206");
    req.param("status", "accepted");
    req.param("per_page", "100"); // max 100 per page how dirty
    req.header("Content-Type", "application/json");

    req.param("page", std::to_string(page));
    m_listener.setFilter(req.get("https://api.geode-sdk.org/v1/mods"));
}


bool MPRankPopup::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto size = m_mainLayer->getContentSize();
    bool geodeTheme = Loader::get()->getLoadedMod("geode.loader")->getSettingValue<bool>("enable-geode-theme");

    this->setTitle("Modder Points Leaderboard");

    auto bg = CCScale9Sprite::create("square02b_001.png");
    bg->setPosition({ m_bgSprite->getPositionX() , m_bgSprite->getPositionY() - 10.f });
    bg->setScale(0.3f);
    bg->setContentSize({ 340.f / 0.3f , 220.f / 0.3f });
    bg->setColor({ 0 , 0 , 0 });
    bg->setOpacity(75);
    bg->setID("scrolllayer-background");
    m_mainLayer->addChild(bg);
    m_bg = bg;

    auto loadingCircle = LoadingCircle::create();
    loadingCircle->setContentSize({ 0 , 0 });
    loadingCircle->setPosition(bg->getPosition());
    loadingCircle->setVisible(false);
    loadingCircle->m_sprite->setPosition({ 0 , 0 });
    loadingCircle->m_sprite->runAction(CCRepeatForever::create(CCRotateBy::create(1, 360)));
    loadingCircle->setID("loading-circle");
    m_mainLayer->addChild(loadingCircle);
    m_loadingCircle = loadingCircle;

    auto failMenu = CCMenu::create();
    failMenu->setScale(0.5f);
    failMenu->setContentSize({ bg->getScaledContentWidth() * 3 / 2 , bg->getScaledContentHeight() * 2});
    failMenu->setPosition(bg->getPosition());
    failMenu->setVisible(false);
    failMenu->setID("status-menu");
    failMenu->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::Center)
            ->setCrossAxisAlignment(AxisAlignment::Center)
            ->setCrossAxisLineAlignment(AxisAlignment::Center)
            ->setGap(5.0f)
            ->setAutoScale(false)
    );
    failMenu->addChild(
        CCLabelBMFont::create(
            "Error loading modders",
            "bigFont.fnt"
        )
    );
    failMenu->addChild(
        CCMenuItemSpriteExtra::create(
            ButtonSprite::create(
                "Retry",
                "bigFont.fnt",
                (geodeTheme ? "geode.loader/GE_button_05.png" : "GJ_button_01.png")
            ),
            this,
            menu_selector(MPRankPopup::onRetryLoading)
        )
    );
    failMenu->updateLayout();
    m_mainLayer->addChild(failMenu);
    m_failMenu = failMenu;


    if (!DevsDataState::isFinished && DevsDataState::isInProgress) {
        loadingCircle->setVisible(true);
    } else if (!DevsDataState::isFinished && DevsDataState::isInternetSilly) {
        failMenu->setVisible(true);
    }

    auto scrollLayer = ScrollLayer::create({ 330.f, 210.f });
    scrollLayer->setPosition({ m_bgSprite->getPositionX() - scrollLayer->getScaledContentWidth() / 2 , m_bgSprite->getPositionY() - 10.f - scrollLayer->getScaledContentHeight() / 2});
    scrollLayer->m_contentLayer->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setAutoGrowAxis(size.height)
            ->setGap(5.0f)
    );
    m_mainLayer->addChild(scrollLayer);
    m_scrollLayer = scrollLayer;

    auto menu = CCMenu::create();
    menu->setPosition({ 25.f , 140.f });
    menu->setContentSize({ 30.f , 200.f });
    menu->setID("left-menu");
    menu->setLayout(
        ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setGap(5.0f)
            ->setAutoScale(false)
    );
    m_mainLayer->addChild(menu);

    auto sdSpriteOff = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("GJ_smallModeIcon_001.png"),
        40, false, 40,
        (geodeTheme ? "GJ_button_02.png" : "GJ_button_01.png"),
        1.f
    );
    auto sdSpriteOn = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("GJ_smallModeIcon_001.png"),
        40, false, 40,
        (geodeTheme ? "geode.loader/GE_button_05.png" : "GJ_button_02.png"),
        1.f
    );
    sdSpriteOff->m_subSprite->setPositionY(sdSpriteOff->m_subSprite->getPositionY() - 2.f);
    sdSpriteOn->m_subSprite->setPositionY(sdSpriteOn->m_subSprite->getPositionY() - 2.f);

    auto scaleDownToggler = CCMenuItemToggler::create(
        sdSpriteOff,
        sdSpriteOn,
        this,
        menu_selector(MPRankPopup::onScaleDown)
    );
    scaleDownToggler->toggle(Mod::get()->getSavedValue<bool>("scale-down", false));
    scaleDownToggler->setScale(0.7f);
    scaleDownToggler->setID("scale-down-toggler");
    menu->addChild(scaleDownToggler);

    auto goldSpriteOff = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("geode.loader/geode-logo-outline.png"),
        40, false, 40,
        (geodeTheme ? "GJ_button_02.png" : "GJ_button_01.png"),
        0.9f
    );
    auto goldSpriteOn = ButtonSprite::create(
        CCSprite::createWithSpriteFrameName("geode.loader/geode-logo-outline-gold.png"),
        40, false, 40,
        (geodeTheme ? "geode.loader/GE_button_05.png" : "GJ_button_02.png"),
        0.9f
    );
    goldSpriteOff->m_subSprite->setPositionY(goldSpriteOff->m_subSprite->getPositionY() - 2.f);
    goldSpriteOn->m_subSprite->setPositionY(goldSpriteOn->m_subSprite->getPositionY() - 2.f);

    auto goldToggler = CCMenuItemToggler::create(
        goldSpriteOff,
        goldSpriteOn,
        this,
        menu_selector(MPRankPopup::onCountFeatured)
    );
    goldToggler->toggle(Mod::get()->getSavedValue<bool>("count-featured", false));
    goldToggler->setScale(0.7f);
    goldToggler->setID("count-featured-toggler");
    menu->addChild(goldToggler);

    menu->updateLayout();

    if (DevsDataState::isFinished) MPRankPopup::loadLeaderboard();

    return true;
}

MPRankPopup* MPRankPopup::create() {
    auto ret = new MPRankPopup();
    if (ret->init(440.f, 290.f)) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}