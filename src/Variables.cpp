#include "Variables.hpp"

matjson::Value DevsData::gDevelopers = matjson::Object();
matjson::Value DevsData::gAliases = matjson::Object();

bool DevsDataState::isFinished = false;
bool DevsDataState::isInProgress = false;
bool DevsDataState::isInternetSilly = false;

uintptr_t Addresses::ModItem_onDevelopers = 0x0;
uintptr_t Addresses::CustomMenuLayer_onGeode = 0x0;

bool Stuff::isOnGeode = false;
int Stuff::thing = 0;