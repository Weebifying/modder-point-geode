#include <Geode/Geode.hpp>
using namespace geode::prelude;

bool compareCount(std::pair<std::string,matjson::Value> a, std::pair<std::string,matjson::Value> b) {
	auto countFeatured = Mod::get()->getSavedValue<bool>("count-featured", false);
	auto resA = countFeatured ? a.second["count"].as_int() + a.second["featured"].as_int() : a.second["count"].as_int();
	auto resB = countFeatured ? b.second["count"].as_int() + b.second["featured"].as_int() : b.second["count"].as_int();
	return resA > resB;
}