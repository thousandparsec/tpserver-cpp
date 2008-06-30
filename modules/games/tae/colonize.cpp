/*  Colonize order
 *
 *  Copyright (C) 2008 Dustin White and the Thousand Parsec Project
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

#include <cassert>
#include <sstream>

#include <tpserver/frame.h>
#include <tpserver/objectorderparameter.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/message.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/logging.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>

#include "ownedobject.h"
#include "starsystem.h"
#include "fleet.h"
#include "planet.h"

#include "colonize.h"

using std::set;
using std::string;
using std::stringstream;

Colonize::Colonize(bool mining) : FleetOrder() {
    isMining = mining;
    if(mining) {
        name = "ColonizeMining";
        description = "Colonize a given system with Mining Robots";
    } else {
        name = "Colonize";
        description = "Colonize a given system";
    }
}

Colonize::~Colonize() {

}

Order* Colonize::clone() const {
    Colonize* o = new Colonize(isMining);
    o->type = type;
    return o;
}

void Colonize::createFrame(Frame *f, int pos) {
    FleetOrder::createFrame(f, pos);
}

Result Colonize::inputFrame(Frame *f, uint32_t playerid) {
    Result r = FleetOrder::inputFrame(f, playerid);
    if(!r) return r;

    Game *game = Game::getGame();
    ObjectManager *obm = game->getObjectManager();
    ObjectTypeManager *obtm = game->getObjectTypeManager();

    IGObject *starSysObj = obm->getObject(starSys->getObjectId());
    StarSystem* starSysData = (StarSystem*) starSysObj->getObjectBehaviour();

    // Check to see if it is a legal system to colonize
    if(starSysObj->getType() == obtm->getObjectTypeByName("Star System") && !starSysData->canBeColonized(isMining)) {
        starSys->setObjectId(0);
        Logger::getLogger()->debug("Player tried to colonize a system which cannot be colonized.");
    }        

    return Success();
}

bool Colonize::doOrder(IGObject * obj) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();
    Fleet* fleetData = (Fleet*)(obj->getObjectBehaviour());
    Player* player = Game::getGame()->getPlayerManager()->getPlayer(fleetData->getOwner());

    IGObject *newStarSys = obm->getObject(starSys->getObjectId());

    //Perform last minute checks to make sure the system can be colonized
    if(newStarSys->getType() != obtm->getObjectTypeByName("Star System")) {
        Logger::getLogger()->debug("Trying to colonize to an object which is not a star system");
        return false;
    }
    if(!((StarSystem*)(newStarSys->getObjectBehaviour()))->canBeColonized(isMining)) {
        Logger::getLogger()->debug("Player tried to colonize a system which cannot be colonized.");
        return false;
    }   

    //Find the star system's planet
    set<uint32_t> children = newStarSys->getContainedObjects();
    uint32_t pid;
    bool planetFound = false;
    for(set<uint32_t>::iterator i=children.begin(); i != children.end(); i++) {
        if(obm->getObject(*i)->getType() == obtm->getObjectTypeByName("Planet")) {
            pid = *i;
            planetFound = true;
        }
    }
    if(!planetFound) {
        Logger::getLogger()->debug("Colonize Order: No planet found in target star system");
        return false;
    }

    //Add resource to planet
    Planet* planet = (Planet*)(obm->getObject(pid)->getObjectBehaviour());
    Design* ship = Game::getGame()->getDesignStore()->getDesign(fleetData->getShips().begin()->first);
    uint32_t scoreType = 0;
    string leaderName;
    if(ship->getName().compare("MerchantShip") == 0) {
        planet->addResource(4, 1);
        scoreType = 1;
        leaderName = "MerchantLeaderShip";
    } else if(ship->getName().compare("ScientistShip") == 0) {
        planet->addResource(5, 1);
        scoreType = 2;
        leaderName = "ScientistLeaderShip";
    } else if(ship->getName().compare("SettlerShip") == 0) {
        planet->addResource(6, 1);
        scoreType = 3;
        leaderName = "SettlerLeaderShip";
    } else if(ship->getName().compare("MiningShip") == 0) {
        planet->addResource(7, 1);
        scoreType = 4;
        leaderName = "MiningLeaderShip";
    }

    //Get bordering star systems' regions
    StarSystem* starSysData = (StarSystem*)(newStarSys->getObjectBehaviour());
    set<uint32_t> regions = getBorderingRegions();
    
    //Put the newly colonized ship into the proper region
    //If it does not border any regions, then set it to a new one with a unique id
    if(regions.size() == 0) {
        starSysData->setRegion(starSys->getObjectId());
        stringstream out;
        out << starSysData->getRegion();
        Logger::getLogger()->debug(string("System " + newStarSys->getName() + " added to region " + out.str()).c_str());
    } else if(regions.size() == 1) {
        // Add +1 to the resource score of the player with the correct leader 
        // in this region
        
        //find the leader type
        uint32_t leaderType;
        uint32_t settlerLeader;
        DesignStore* ds = Game::getGame()->getDesignStore();
        set<unsigned int> designs = ds->getDesignIds(); 
        for(set<unsigned int>::iterator i = designs.begin(); i != designs.end(); i++) {
            if(ds->getDesign(*i)->getName().compare(leaderName) == 0) {
                leaderType = *i;
            } 
            if(ds->getDesign(*i)->getName().compare("SettlerLeaderShip") == 0) {
                settlerLeader = *i;
            }
        }
        //find the leader of the region... add 1 to the player's score
        int leaderID = getLeaderInRegion(*(regions.begin()), leaderType);
        if(leaderID < 0 && leaderType != settlerLeader) {
            leaderID = getLeaderInRegion(*(regions.begin()), settlerLeader);
        }
        if(leaderID >= 0) {
            Fleet* leader = (Fleet*) ((obm->getObject((uint32_t) leaderID))->getObjectBehaviour());
            Player* owner = Game::getGame()->getPlayerManager()->getPlayer(leader->getOwner());
            owner->setScore(scoreType, owner->getScore(scoreType) + 1);
        }    

        //Add the newly colonized system to the region
        starSysData->setRegion(*(regions.begin()));
        stringstream out;
        out << starSysData->getRegion();
        Logger::getLogger()->debug(string("System " + newStarSys->getName() + " added to region " + out.str()).c_str());
    } else {
        //TODO: This means that 2 regions are being connected. Perform check
        //      For external combat here then perform region check again!
    }

    obm->doneWithObject(newStarSys->getID());
   
 
    // post completion message
    Message * msg = new Message();
    msg->setSubject("Colonize fleet order complete");
    msg->setBody(string("You're fleet, \"" + obj->getName() + "\" has colonized ")
            + newStarSys->getName() + ".");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, starSys->getObjectId());
    msg->addReference(rst_Object, obj->getID());

    player->postToBoard(msg);

    //Remove fleet
    obj->removeFromParent();
    obm->scheduleRemoveObject(obj->getID());

    return true;
}
