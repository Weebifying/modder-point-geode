#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
using namespace geode::prelude;

class MPRankItem : public cocos2d::CCNode {
protected:
	int m_rank;
	std::string m_name;
	int m_count;
private:
	bool init(int rank, std::string name, int count, bool scaleDown, bool countFeatured);
public:
	static MPRankItem* create(int rank, std::string name, int count, bool scaleDown, bool countFeatured);
};

class MPRankPopup : public geode::Popup<> {
protected:
	CCScale9Sprite* m_bg;
	ScrollLayer* m_scrollLayer;
	EventListener<web::WebTask> m_listener;

	bool init(float width, float height);

    bool setup();

public:
	LoadingCircle* m_loadingCircle;
	CCMenu* m_failMenu;

    static MPRankPopup* create();

	void loadLeaderboard();
	void onScaleDown(CCObject* sender);
	void onCountFeatured(CCObject* sender);
	void onRetryLoading(CCObject* sender);
};