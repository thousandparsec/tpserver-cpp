/*  TaE ruleset
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//System includes
#include <sstream>

//tpserver includes
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/player.h>

//tae includes
#include "universe.h"
#include "emptyobject.h"
#include "spaceobject.h"

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
    Game* game = Game::getGame();

    ObjectTypeManager* obtm = game->getObjectTypeManager();
    obtm->addNewObjectType(new UniverseType());
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
