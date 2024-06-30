#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/utils/web.hpp>
using namespace geode::prelude;

matjson::Value gDevelopers = matjson::Object();
matjson::Value gAliases = matjson::Object();

uintptr_t onDevelopersAddress = 0x0;
uintptr_t onGeodeAddress = 0x0;
bool isOnGeode = false;
int thing = 0;

bool compareCount(std::pair<std::string,matjson::Value> a, std::pair<std::string,matjson::Value> b) {
	auto countFeatured = Mod::get()->getSavedValue<bool>("count-featured", false);
	auto resA = countFeatured ? a.second["count"].as_int() + a.second["featured"].as_int() : a.second["count"].as_int();
	auto resB = countFeatured ? b.second["count"].as_int() + b.second["featured"].as_int() : b.second["count"].as_int();
	return resA > resB;
}

class MPRankItem : public cocos2d::CCNode {
protected:
	int m_rank;
	std::string m_name;
	int m_count;
private:
	bool init(int rank, std::string name, int count, bool scaleDown, bool countFeatured) {
		if (!CCNode::init()) return false;

		bool geodeTheme = Loader::get()->getLoadedMod("geode.loader")->getSettingValue<bool>("enable-geode-theme");
		this->setContentSize({ 330.f , (scaleDown ? 20.f : 40.f) });

		auto menuBG = CCScale9Sprite::create("square02b_001.png");
		menuBG->setScale(0.3f);
		menuBG->setContentSize({ 330.f / 0.3f , (scaleDown ? 20.f : 40.f) / 0.3f });
		menuBG->setColor(geodeTheme ? ccColor3B { 255 , 255 , 255 } : ccColor3B { 0 , 0 , 0 });
		menuBG->setOpacity(geodeTheme ? 15 : 50);
		menuBG->setPosition({ this->getContentWidth() / 2 , this->getContentHeight() / 2});
		this->addChild(menuBG);

		auto label = CCLabelBMFont::create((std::to_string(rank) + std::string(". ") + name).c_str(), "goldFont.fnt");
		label->setScale(scaleDown ? 0.4f : 0.8f);
		label->setAnchorPoint({ 0 , 0.5 });
		label->setPosition({ 5.f, this->getContentHeight() / 2 });
		this->addChild(label);

		auto geodeSprite = CCSprite::createWithSpriteFrameName(
			countFeatured ? "geode.loader/geode-logo-outline-gold.png" : "geode.loader/geode-logo-outline.png"
		);
		geodeSprite->setScale(scaleDown ? 0.4f : 0.8f);
		geodeSprite->setAnchorPoint({ 1, 0.5 });
		geodeSprite->setPosition({ this->getContentWidth() - 5.f , this->getContentHeight() / 2 });
		this->addChild(geodeSprite);

		auto countLabel = CCLabelBMFont::create(std::to_string(count).c_str(), "bigFont.fnt");
		countLabel->setScale(scaleDown ? 0.4f : 0.8f);
		countLabel->setAnchorPoint({ 1 , 0.5 });
		countLabel->setPosition({ this->getContentWidth() - geodeSprite->getScaledContentWidth() - 10.f , this->getContentHeight() / 2 });
		this->addChild(countLabel);

		return true;
	}

public:
	static MPRankItem* create(int rank, std::string name, int count, bool scaleDown, bool countFeatured) {
		auto ret = new MPRankItem();
		if (ret->init(rank, name, count, scaleDown, countFeatured)) {
			ret->autorelease();
			return ret;
		}

		delete ret;
		return nullptr;
	}
};

class MPRankPopup : public geode::Popup<> {
protected:
	CCScale9Sprite* m_bg;
	ScrollLayer* m_scrollLayer;

	bool init(float width, float height) {
		// straight up copied from GeodePopup definition
		bool geodeTheme = Loader::get()->getLoadedMod("geode.loader")->getSettingValue<bool>("enable-geode-theme");
        const char* bg = geodeTheme ? "geode.loader/GE_square01.png" : "GJ_square01.png";

		if (!Popup<>::initAnchored(width, height, bg))
            return false;

		this->setCloseButtonSpr(
			CircleButtonSprite::createWithSpriteFrameName(
				"geode.loader/close.png", .85f,
				(geodeTheme ? CircleBaseColor::DarkPurple : CircleBaseColor::Green)
			)
		);

		return true;
	}

	void loadLeaderboard() {
		int c = 1;
		sort(gDevelopers.as_object().begin(), gDevelopers.as_object().end(), compareCount);

		for (auto& [key, value] : gDevelopers.as_object()) {
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

	void onScaleDown(CCObject* sender) {
		Mod::get()->setSavedValue<bool>("scale-down", !as<CCMenuItemToggler*>(sender)->m_toggled);
		m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);

		MPRankPopup::loadLeaderboard();
	}

	void onCountFeatured(CCObject* sender) {
		Mod::get()->setSavedValue<bool>("count-featured", !as<CCMenuItemToggler*>(sender)->m_toggled);
		m_scrollLayer->m_contentLayer->removeAllChildrenWithCleanup(true);

		MPRankPopup::loadLeaderboard();
	}


    bool setup() override {
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
		m_mainLayer->addChild(bg);
		m_bg = bg;

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
		auto scaleDownToggler = CCMenuItemToggler::create(
			sdSpriteOff,
			sdSpriteOn,
			this,
			menu_selector(MPRankPopup::onScaleDown)
		);
		scaleDownToggler->toggle(Mod::get()->getSavedValue<bool>("scale-down", false));
		scaleDownToggler->setScale(0.7f);
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
		auto goldToggler = CCMenuItemToggler::create(
			goldSpriteOff,
			goldSpriteOn,
			this,
			menu_selector(MPRankPopup::onCountFeatured)
		);
		goldToggler->toggle(Mod::get()->getSavedValue<bool>("count-featured", false));
		goldToggler->setScale(0.7f);
		menu->addChild(goldToggler);

		menu->updateLayout();
		MPRankPopup::loadLeaderboard();

        return true;
    }

public:
    static MPRankPopup* create() {
        auto ret = new MPRankPopup();
        if (ret->init(440.f, 290.f)) {
            ret->autorelease();
            return ret;
        }

        delete ret;
        return nullptr;
    }
};


void ModItem_onDevelopers(CCNode* self, CCObject* sender) {
    ((void(*)(CCNode*, CCObject*))onDevelopersAddress)(self, sender); // trampoline
	
	if (auto popup = getChildOfType<FLAlertLayer>(CCDirector::get()->getRunningScene(), 0)) {
		auto layer = getChildOfType<CCLayer>(popup, 0);
		CCArrayExt<CCNode*> children = layer->getChildren();

		for (auto& node : children) {
			if (node->getZOrder() == 0 && node->getChildrenCount() == 2) {
				if (auto scrollLayer = getChildOfType<ScrollLayer>(node, 0)) {
					if (auto genericContentLayer = getChildOfType<GenericContentLayer>(scrollLayer, 0)) {
						CCArrayExt<CCNode*> subchildren = genericContentLayer->getChildren();

						for(auto& devItem : subchildren) {
							auto label = getChildOfType<CCLabelBMFont>(devItem, 0);
							auto name = label->getString();

							auto geodeSprite = CCSprite::createWithSpriteFrameName("geode.loader/geode-logo-outline.png");
							geodeSprite->setScale(0.4f);
							geodeSprite->setPositionY(devItem->getContentHeight() / 2.f);
							geodeSprite->setAnchorPoint({ 1.f , 0.5f });

							if (gDevelopers.contains(name)) {
								auto countLabel = CCLabelBMFont::create(std::to_string(gDevelopers[name]["count"].as_int()).c_str(), "bigFont.fnt");
								countLabel->setScale(0.4f);
								countLabel->setPositionY(devItem->getContentHeight() / 2.f);
								countLabel->setAnchorPoint({ 1.f , 0.5f });

								countLabel->setPositionX(devItem->getContentWidth() - 5.f);

								if (auto menu = getChildOfType<CCMenu>(devItem, 0)) {
									countLabel->setPositionX(countLabel->getPositionX() - menu->getScaledContentWidth() / 2.f - 10.f);
								}

								geodeSprite->setPositionX(countLabel->getPositionX() - countLabel->getScaledContentWidth() - 5.f);

								devItem->addChild(countLabel);
								devItem->addChild(geodeSprite);

							} else if (gAliases.contains(name)) {
								auto countLabel = CCLabelBMFont::create(std::to_string(gDevelopers[gAliases[name].as_string()]["count"].as_int()).c_str(), "bigFont.fnt");
								countLabel->setScale(0.4f);
								countLabel->setPositionY(devItem->getContentHeight() / 2.f);
								countLabel->setAnchorPoint({ 1.f , 0.5f });

								countLabel->setPositionX(devItem->getContentWidth() - 5.f);

								if (auto menu = getChildOfType<CCMenu>(devItem, 0)) {
									countLabel->setPositionX(countLabel->getPositionX() - menu->getScaledContentWidth() / 2.f - 10.f);
								}

								geodeSprite->setPositionX(countLabel->getPositionX() - countLabel->getScaledContentWidth() - 5.f);

								devItem->addChild(countLabel);
								devItem->addChild(geodeSprite);
							}
						}
					}
				}
			}
		}
	}
}

class $modify(MyDirector, CCDirector) {
	void onMPRank(CCObject* sender) {
		MPRankPopup::create()->show();
	}

	// do NOT ask
	bool replaceScene(CCScene* scene) {
		if (!CCDirector::replaceScene(scene)) return false;
		if (isOnGeode) thing++;
		if (thing == 2) {

			CCArrayExt<CCNode*> children = scene->getChildren();

			// get ModItem::onDevelopers address
			if (auto layer = scene->getChildByID("ModsLayer")) {
				auto winSize = CCDirector::get()->getWinSize();
				bool geodeTheme = Loader::get()->getLoadedMod("geode.loader")->getSettingValue<bool>("enable-geode-theme");

				auto rankMenu = CCMenu::create();
				rankMenu->setPosition({ winSize.width - 10 , 10 });
				rankMenu->setContentSize({ 46.5 , 200 });
				rankMenu->setAnchorPoint({ 1 , 0 });
				rankMenu->setLayout(
					ColumnLayout::create()
						->setAxisAlignment(AxisAlignment::Start)
						->setCrossAxisAlignment(AxisAlignment::Center)
						->setCrossAxisLineAlignment(AxisAlignment::Center)
				);
				rankMenu->setID("mp-rank-menu"_spr);
				layer->addChild(rankMenu);
				layer->updateLayout();

				auto rankButton = CCMenuItemSpriteExtra::create(
					CircleButtonSprite::createWithSpriteFrameName(
						"rankIcon_top10_001.png", 
						1.2f, 
						geodeTheme ? CircleBaseColor::DarkPurple : CircleBaseColor::Green, 
						CircleBaseSize::Medium
					),
					layer,
					menu_selector(MyDirector::onMPRank)
				);
				rankButton->setPosition({ rankButton->getScaledContentWidth() / 2 , rankButton->getScaledContentHeight() / 2 });
				rankButton->setID("mp-rank-button"_spr);
				rankMenu->addChild(rankButton);

				if (auto listFrame = layer->getChildByID("mod-list-frame")) {
					if (auto list = listFrame->getChildByID("ModList")) {
						if (auto scrollLayer = list->getChildByID("ScrollLayer")) {
							if (auto contentLayer = scrollLayer->getChildByID("content-layer")) {
								if (auto modItem = contentLayer->getChildByID("ModItem")) {
									if (auto container = modItem->getChildByID("info-container")) {
										if (auto menu = container->getChildByID("developers-menu")) {
											if (auto button = menu->getChildByID("developers-button")) {
												auto selector = as<CCMenuItemSpriteExtra*>(button)->m_pfnSelector;
												onDevelopersAddress = addresser::getNonVirtual(selector);
											} // LOL
										}
									}
								}
							}
						}
					}
				}
			}

			if (onDevelopersAddress != 0x0) {
				auto INT_MIN_Hook = tulip::hook::HookMetadata();
				INT_MIN_Hook.m_priority = INT_MIN;

				for (auto& hook : Mod::get()->getHooks()) {
					if (hook->getDisplayName() == "ModItem::onDevelopers") {
						return true;
					}
				}

				Mod::get()->hook(
					reinterpret_cast<void*>(onDevelopersAddress),
					&ModItem_onDevelopers,
					"ModItem::onDevelopers",
					tulip::hook::TulipConvention::Thiscall,
					INT_MIN_Hook
				);
			}

			isOnGeode = false;
			thing = 0;
		}

		return true;
	}
};

class $modify(MyLoadingLayer, LoadingLayer) {
    struct Fields {
        EventListener<web::WebTask> m_listener;
        int m_page = 1;
    };

    bool init(bool p0) {
        if (!LoadingLayer::init(p0)) return false;

        matjson::Value developers;
		matjson::Value aliases;

		log::error("LoadingLayer::init");

        m_fields->m_listener.bind([this, developers, aliases] (web::WebTask::Event* e) mutable {
            if (web::WebResponse* res = e->getValue()) {
                developers = gDevelopers;
				aliases = gAliases;

                auto data = res->json().unwrapOr("Uh oh!")["payload"]["data"];

                for (auto& mod : data.as_array()) {
                    matjson::Array devs = mod["developers"].as_array();
                    for (auto& dev : devs) {
                        if (dev["is_owner"].as_bool()) {
                            auto displayName = dev["display_name"].as_string();

							if (Loader::get()->isModInstalled(mod["id"].as_string())) {
								auto leMod = Loader::get()->getInstalledMod(mod["id"].as_string());
								if (leMod->getDeveloper() != displayName) {
									if (!aliases.contains(leMod->getDeveloper())) {
										aliases[leMod->getDeveloper()] = displayName;
									} else if (aliases[leMod->getDeveloper()] != displayName) {
										log::warn("bro what? {}", leMod->getDeveloper());
									}
								}
							}

                            if (!developers.contains(displayName)) {
                                developers[displayName] = matjson::Object();
                                developers[displayName]["username"] = dev["username"].as_string();
                                developers[displayName]["count"] = 0;
								developers[displayName]["featured"] = 0;
                            }
							
							// lmao this is disgusting
                            developers[displayName]["count"] = developers[displayName]["count"].as_int() + 1;
							developers[displayName]["featured"] = developers[displayName]["featured"].as_int() + (mod["featured"].as_bool() ? 1 : 0);

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

                    req.param("page", std::to_string(++m_fields->m_page));

                    m_fields->m_listener.setFilter(req.get("https://api.geode-sdk.org/v1/mods"));
                }

                gDevelopers = developers;
				gAliases = aliases;

            } else if (web::WebProgress* p = e->getProgress()) {
                log::info("Progress: {}", p->downloadProgress().value_or(0.f));
            } else if (e->isCancelled()) {
                log::info("The request was cancelled... So sad :(");
            }
        });

        web::WebRequest req = web::WebRequest();

        req.param("gd", "2.206");
        req.param("status", "accepted");
        req.param("per_page", "100"); // max 100 per page how dirty
        req.header("Content-Type", "application/json");

        req.param("page", std::to_string(m_fields->m_page));
        m_fields->m_listener.setFilter(req.get("https://api.geode-sdk.org/v1/mods"));

        return true;
    }
};

void CustomMenuLayer_onGeode(MenuLayer* self, CCObject* sender) {
	isOnGeode = true;
	thing = 0;
	((void(*)(MenuLayer*, CCObject*))onGeodeAddress)(self, sender); // trampoline
}

class $modify(MenuLayer) {
	static void onModify(auto& self) {
        self.setHookPriority("MenuLayer::init", INT_MIN);
    }

	bool init() {
		if (!MenuLayer::init()) return false;

		// get CustomMenuLayer::onGeode address
		if (Loader::get()->isModLoaded("alphalaneous.pages_api")) {
			if (auto menu = this->getChildByID("bottom-menu")) {
				if (auto pages = menu->getChildByID("pages")) {
					CCArrayExt<CCMenu*> page = pages->getChildren();
					for (auto& pMenu : page) {
						CCArrayExt<CCMenuItemSpriteExtra*> buttons = pMenu->getChildren();
						for (auto& button : buttons) {
							if (button->getID() == "geode.loader/geode-button") {
								auto selector = button->m_pfnSelector;
								onGeodeAddress = addresser::getNonVirtual(selector);
							}
						}
					}
				}
			}
		} else {
			log::info("hey");
			if (auto menu = this->getChildByID("bottom-menu")) {
				log::warn("found menu");
				if (auto button = menu->getChildByID("geode.loader/geode-button")) {
					log::error("found button");
					onGeodeAddress = addresser::getNonVirtual(as<CCMenuItemSpriteExtra*>(button)->m_pfnSelector);
				}
			}
		}

		if (onGeodeAddress != 0x0) {
			auto INT_MIN_Hook = tulip::hook::HookMetadata();
			INT_MIN_Hook.m_priority = INT_MIN;

			for (auto& hook : Mod::get()->getHooks()) {
				if (hook->getDisplayName() == "CustomMenuLayer::onGeode") {
					return true;
				}
			}

			Mod::get()->hook(
				reinterpret_cast<void*>(onGeodeAddress),
				&CustomMenuLayer_onGeode,
				"CustomMenuLayer::onGeode",
				tulip::hook::TulipConvention::Thiscall,
				INT_MIN_Hook
			);
		}

		return true;
	}
};