
//System includes
#include <sstream>

//tpserver includes
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/player.h>

//tae includes

//header includes
#include "tae.h"

using namespace tae;

taeRuleset::taeRuleset() {
}

taeRuleset::~taeRuleset() {
}

std::string taeRuleset::getName() {
    return "TaE";
}

std::string taeRuleset::getVersion() {
    return "0.01";
}

void taeRuleset::initGame() {
    Logger::getLogger()->info("TaE initialised");
}

void taeRuleset::createGame() {
    Logger::getLogger()->info("TaE created");
}

void taeRuleset::startGame() {
    Logger::getLogger()->info("TaE started");
}

bool taeRuleset::onAddPlayer(Player* player) {
    Logger::getLogger()->debug("TaE onAddPlayer");
    return true;
}

void taeRuleset::onPlayerAdded(Player* player) {
    Logger::getLogger()->debug("TaE onPlayerAdded");
}

extern "C" {
    #define tp_init libtae_LTX_tp_init
    bool tp_init() {
        return Game::getGame()->setRuleset(new tae::taeRuleset());
    }
}
