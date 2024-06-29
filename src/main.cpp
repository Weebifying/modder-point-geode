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

							auto geodeSprite = CCSprite::createWithSpriteFrameName("geode.loader/geode-logo-outline-gold.png");
								geodeSprite->setScale(0.4f);
								geodeSprite->setPositionY(devItem->getContentHeight() / 2);
								geodeSprite->setAnchorPoint({ 1, 0.5 });

							if (gDevelopers.contains(name)) {
								auto countLabel = CCLabelBMFont::create(std::to_string(gDevelopers[name]["count"].as_int()).c_str(), "bigFont.fnt");
								countLabel->setScale(0.4f);
								countLabel->setPositionY(devItem->getContentHeight() / 2);
								countLabel->setAnchorPoint({ 1, 0.5 });

								countLabel->setPositionX(devItem->getContentWidth() - 5);

								if (auto menu = getChildOfType<CCMenu>(devItem, 0)) {
									countLabel->setPositionX(countLabel->getPositionX() - menu->getScaledContentWidth() / 2 - 10);
								}

								geodeSprite->setPositionX(countLabel->getPositionX() - countLabel->getScaledContentWidth() - 5);

								devItem->addChild(countLabel);
								devItem->addChild(geodeSprite);

							} else if (gAliases.contains(name)) {
								auto countLabel = CCLabelBMFont::create(std::to_string(gDevelopers[gAliases[name].as_string()]["count"].as_int()).c_str(), "bigFont.fnt");
								countLabel->setScale(0.4f);
								countLabel->setPositionY(devItem->getContentHeight() / 2);
								countLabel->setAnchorPoint({ 1, 0.5 });

								countLabel->setPositionX(devItem->getContentWidth() - 5);

								if (auto menu = getChildOfType<CCMenu>(devItem, 0)) {
									countLabel->setPositionX(countLabel->getPositionX() - menu->getScaledContentWidth() / 2 - 10);
								}

								geodeSprite->setPositionX(countLabel->getPositionX() - countLabel->getScaledContentWidth() - 5);

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

class $modify(CCDirector) {
	// do NOT ask
	bool replaceScene(CCScene* scene) {
		if (!CCDirector::replaceScene(scene)) return false;
		if (isOnGeode) thing++;
		if (thing == 2) {

			CCArrayExt<CCNode*> children = scene->getChildren();

			// get ModItem::onDevelopers address
			if (auto layer = scene->getChildByID("ModsLayer")) {
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
                            }

                            int i = developers[displayName]["count"].as_int();
                            developers[displayName]["count"] = i+1;

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
                // log::info("progress: {}", p->downloadProgress().value_or(0.f));
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
			if (auto menu = this->getChildByID("bottom-menu")) {
				if (auto button = menu->getChildByID("geode.loader/geode-button")) {
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