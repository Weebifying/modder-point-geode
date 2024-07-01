#include <Geode/Geode.hpp>
using namespace geode::prelude;

namespace DevsData {
    extern matjson::Value gDevelopers;
    extern matjson::Value gAliases;
};

namespace DevsDataState {
    extern bool isFinished;
    extern bool isInProgress;
    extern bool isInternetSilly;
};

namespace Addresses {
    extern uintptr_t ModItem_onDevelopers;
    extern uintptr_t CustomMenuLayer_onGeode;
};

namespace Stuff {
    extern bool isOnGeode;
    extern int thing;
};