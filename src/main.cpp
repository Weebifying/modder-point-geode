#include "MPRank.hpp"
#include "Variables.hpp"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/utils/web.hpp>

void ModItem_onDevelopers(CCNode* self, CCObject* sender) {
    ((void(*)(CCNode*, CCObject*))Addresses::ModItem_onDevelopers)(self, sender); // trampoline
	
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

							if (DevsData::gDevelopers.contains(name)) {
								auto countLabel = CCLabelBMFont::create(std::to_string(DevsData::gDevelopers[name]["count"].as_int()).c_str(), "bigFont.fnt");
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

							} else if (DevsData::gAliases.contains(name)) {
								auto countLabel = CCLabelBMFont::create(std::to_string(DevsData::gDevelopers[DevsData::gAliases[name].as_string()]["count"].as_int()).c_str(), "bigFont.fnt");
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
		if (Stuff::isOnGeode) Stuff::thing++;
		if (Stuff::thing == 2) {

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
												Addresses::ModItem_onDevelopers = addresser::getNonVirtual(selector);
											} // LOL
										}
									}
								}
							}
						}
					}
				}
			}

			if (Addresses::ModItem_onDevelopers != 0x0) {
				auto INT_MIN_Hook = tulip::hook::HookMetadata();
				INT_MIN_Hook.m_priority = INT_MIN;

				for (auto& hook : Mod::get()->getHooks()) {
					if (hook->getDisplayName() == "ModItem::onDevelopers") {
						return true;
					}
				}

				Mod::get()->hook(
					reinterpret_cast<void*>(Addresses::ModItem_onDevelopers),
					&ModItem_onDevelopers,
					"ModItem::onDevelopers",
					tulip::hook::TulipConvention::Thiscall,
					INT_MIN_Hook
				);
			}

			Stuff::isOnGeode = false;
			Stuff::thing = 0;
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

        m_fields->m_listener.bind([this] (web::WebTask::Event* e) mutable {
			auto runningScene = CCDirector::get()->getRunningScene();

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

						req.param("page", std::to_string(++m_fields->m_page));

						m_fields->m_listener.setFilter(req.get("https://api.geode-sdk.org/v1/mods"));
					} else {
						DevsDataState::isFinished = true;
						DevsDataState::isInProgress = false;

						if (runningScene) {
							if (auto popup = getChildOfType<MPRankPopup>(runningScene, 0)) {
								popup->m_loadingCircle->setVisible(false);
								popup->loadLeaderboard();
							}
						}
					}
				} else {
					log::error("Your internet connection is silly!");
					DevsDataState::isInternetSilly = true;
					DevsDataState::isInProgress = false;
					if (runningScene) {
						if (auto popup = getChildOfType<MPRankPopup>(runningScene, 0)) {
							popup->m_loadingCircle->setVisible(false);
							popup->m_failMenu->setVisible(true);
						}
					}				
				}

            } else if (web::WebProgress* p = e->getProgress()) {
                // log::info("Progress: {}", p->downloadProgress().value_or(0.f));
				DevsDataState::isInProgress = true;
            } else if (e->isCancelled()) {
                log::warn("The request was cancelled... So sad :(");
				DevsDataState::isInternetSilly = true;
				DevsDataState::isInProgress = false;
				if (runningScene) {
					if (auto popup = getChildOfType<MPRankPopup>(runningScene, 0)) {
						popup->m_loadingCircle->setVisible(false);
						popup->m_failMenu->setVisible(true);
					}
				}
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
	Stuff::isOnGeode = true;
	Stuff::thing = 0;
	((void(*)(MenuLayer*, CCObject*))Addresses::CustomMenuLayer_onGeode)(self, sender); // trampoline
}

class $modify(MenuLayer) {
	static void onModify(auto& self) {
        self.setHookPriority("MenuLayer::init", INT_MIN/2 + 2);
    }

	bool init() {
		if (!MenuLayer::init()) return false;

		// get CustomMenuLayer::onGeode address
		if (auto menu = this->getChildByID("bottom-menu")) {
			if (auto button = menu->getChildByID("geode.loader/geode-button")) {
				Addresses::CustomMenuLayer_onGeode = addresser::getNonVirtual(as<CCMenuItemSpriteExtra*>(button)->m_pfnSelector);
			}
		}

		if (Addresses::CustomMenuLayer_onGeode != 0x0) {
			auto INT_MIN_Hook = tulip::hook::HookMetadata();
			INT_MIN_Hook.m_priority = INT_MIN;

			for (auto& hook : Mod::get()->getHooks()) {
				if (hook->getDisplayName() == "CustomMenuLayer::onGeode") {
					return true;
				}
			}

			Mod::get()->hook(
				reinterpret_cast<void*>(Addresses::CustomMenuLayer_onGeode),
				&CustomMenuLayer_onGeode,
				"CustomMenuLayer::onGeode",
				tulip::hook::TulipConvention::Thiscall,
				INT_MIN_Hook
			);
		}

		return true;
	}
};