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
#include <tpserver/object.h>
#include <tpserver/objectview.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/designview.h>

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

    //Add universe object type
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    obtm->addNewObjectType(new UniverseType());
    Logger::getLogger()->info("TaE initialised");

    //Add Galaxy object type
    EmptyObjectType * eo = new EmptyObjectType();
    eo->setTypeName("Galaxy");
    eo->setTypeDescription("The Galaxy Object type");
    obtm->addNewObjectType(eo);

    //Add Solar system object type
    EmptyObjectType * eo2 = new EmptyObjectType();
    eo2->setTypeName("Solar System");
    eo2->setTypeDescription("The Solar System Object type");
    obtm->addNewObjectType(eo2);
}

void taeRuleset::createGame() {
    Game* game = Game::getGame();
    ObjectManager* obm = game->getObjectManager();
    ObjectTypeManager* obtm = game->getObjectTypeManager();

    uint32_t obT_Universe = obtm->getObjectTypeByName("Universe");
    uint32_t obT_Galaxy = obtm->getObjectTypeByName("Galaxy");
    uint32_t obT_Solar_System = obtm->getObjectTypeByName("Solar System");

    //Create the universe
    IGObject* universe = obm->createNewObject();
    obtm->setupObject(universe, obT_Universe);
    Universe* theUniverse = (Universe*)(universe->getObjectBehaviour());
    theUniverse->setSize(1000000000000ll);
    universe->setName("The Universe");
    theUniverse->setPosition(Vector3d(0ll,0ll,0ll));
    obm->addObject(universe);

    //Create the galaxy
    IGObject* gal = obm->createNewObject();
    obtm->setupObject(gal, obT_Galaxy);
    EmptyObject* galob = (EmptyObject*)(gal->getObjectBehaviour());
    galob->setSize(100000000000ll);
    gal->setName("The Fertile Galaxy");
    galob->setPosition(Vector3d(0ll, -6000ll, 0ll));
    gal->addToParent(universe->getID());
    obm->addObject(gal);

    //Create the "Board" of solar systems
    for(int i = 0; i < 11; i++) {
        for(int j = 0; j < 16; j++) {
            //Create a solar system
            IGObject* sys1 = obm->createNewObject();
            obtm->setupObject(sys1, obT_Solar_System);
            EmptyObject* sys1ob = (EmptyObject*)(sys1->getObjectBehaviour());
            sys1ob->setSize(60000ll);
            char* name = new char[20];
            sprintf(name, "Solar System %d,%d", j, i);
            sys1->setName(name);
            sys1ob->setPosition(Vector3d(1ll + 80000ll*j, 1ll+80000ll*i, 0ll));
            sys1->addToParent(gal->getID());
            obm->addObject(sys1);
        }
    }
    
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

    Game *game = Game::getGame();
    PlayerView* playerview = player->getPlayerView();

    std::set<uint32_t> allotherdesigns = Game::getGame()->getDesignStore()->getDesignIds();
    for(std::set<uint32_t>::const_iterator desid = allotherdesigns.begin(); desid != allotherdesigns.end(); ++desid){
        DesignView* dv = new DesignView();
        dv->setDesignId(*desid);
        dv->setIsCompletelyVisible(true);
        playerview->addVisibleDesign(dv);
    }

    std::set<uint32_t> objids = game->getObjectManager()->getAllIds();
    for(std::set<uint32_t>::iterator itcurr = objids.begin(); itcurr != objids.end(); ++itcurr){
        ObjectView* obv = new ObjectView();
        obv->setObjectId(*itcurr);
        obv->setCompletelyVisible(true);
        playerview->addVisibleObject(obv);
    }
}

extern "C" {
    #define tp_init libtae_LTX_tp_init
    bool tp_init() {
        return Game::getGame()->setRuleset(new tae::taeRuleset());
    }
}
