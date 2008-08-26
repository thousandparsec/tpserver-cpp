/*  The Move class defines a FleetOrder which is used by the leader
 *  fleets to occupy systems.  This may create an internal conflict.
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
#include "fleet.h"
#include "planet.h"
#include "starsystem.h"
#include "taeturn.h"

#include "move.h"

using std::set;
using std::map;
using std::string;
using std::stringstream;

Move::Move() : FleetOrder() {
    name = "Move";
    description = "Move to a given system";
}

Move::~Move() {

}

Order* Move::clone() const {
    Move* o = new Move();
    o->type = type;
    return o;
}

void Move::createFrame(Frame *f, int pos) {
    FleetOrder::createFrame(f, pos);
}

Result Move::inputFrame(Frame *f, uint32_t playerid) {
    Result r = FleetOrder::inputFrame(f, playerid);
    if(!r) return r;

    Game *game = Game::getGame();
    ObjectManager *obm = game->getObjectManager();
    ObjectTypeManager *obtm = game->getObjectTypeManager();

    IGObject *starSysObj = obm->getObject(starSys->getObjectId());
    StarSystem* starSysData = (StarSystem*) starSysObj->getObjectBehaviour();

    // Check to see if it is a legal system for this fleet 
    if(starSysObj->getType() == obtm->getObjectTypeByName("Star System")) {
        if(!starSysData->canBeColonized(false)) {
            starSys->setObjectId(0);
            Logger::getLogger()->debug("Player tried to move a system which cannot be colonized.");
        } else if(getBorderingRegions().size() > 1) {
            starSys->setObjectId(0);
            Logger::getLogger()->debug("Player tried to occupy a system which would join two or more regions.");
        }
    }    

    return Success();
}

bool Move::doOrder(IGObject * obj) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();
    Fleet* fleetData = (Fleet*)(obj->getObjectBehaviour());
    Player* player = Game::getGame()->getPlayerManager()->getPlayer(fleetData->getOwner());

    IGObject* newStarSys = obm->getObject(starSys->getObjectId());

    //Perform last minute checks to make sure this is a valid system
    if(newStarSys->getType() != obtm->getObjectTypeByName("Star System")) {
        Logger::getLogger()->debug("Trying to move to an object which is not a star system");
        Message * msg = new Message();
        msg->setSubject("Move order failed");
        msg->setBody(string("You're fleet, \"" + obj->getName() + "\" tried to move to an object which is not a star system!"));
        msg->addReference(rst_Object, obj->getID());
        player->postToBoard(msg);
        return false;
    }
    StarSystem* starSysData = (StarSystem*)(newStarSys->getObjectBehaviour());
    if(!starSysData->canBeColonized(false)) {
        Logger::getLogger()->debug("Player tried to move a system which cannot be colonized.");
        Message * msg = new Message();
        msg->setSubject("Move order failed");
        msg->setBody(string("You're fleet, \"" + obj->getName() + "\" tried to move to an invalid system!"));
        msg->addReference(rst_Object, obj->getID());
        player->postToBoard(msg);
        return false;
    } else if(getBorderingRegions().size() > 1) {
        Logger::getLogger()->debug("Player tried to occupy a system which would join two or more regions.");
        Message * msg = new Message();
        msg->setSubject("Move order failed");
        msg->setBody(string("You're fleet, \"" + obj->getName() + "\" tried to occupy a system which would join two or more regions!"));
        msg->addReference(rst_Object, obj->getID());
        player->postToBoard(msg);
        return false;
    } else if(!isBorderingScienceColony(starSysData)) {
        Logger::getLogger()->debug("Player tried to occupy a system which does not border a science colony.");
        Message * msg = new Message();
        msg->setSubject("Move order failed");
        msg->setBody(string("You're fleet, \"" + obj->getName() + "\" tried to move to a system which does not border a science colony!"));
        msg->addReference(rst_Object, obj->getID());
        player->postToBoard(msg);
        return false;
    }

    //Set new parent's region
    set<uint32_t> regions = getBorderingRegions();
    //If it does not border any regions, the add it to a new one
    if(regions.size() == 0) {
        starSysData->setRegion(starSys->getObjectId());
        stringstream out;
        out << starSysData->getRegion();
        Logger::getLogger()->debug(string("System " + newStarSys->getName() + " added to region " + out.str()).c_str());
    } else if(regions.size() == 1) {
        //Check for another leader of the fleet type in this region.
        uint32_t leaderType = fleetData->getShips().begin()->first;
        string name = Game::getGame()->getDesignStore()->getDesign(leaderType)->getName();
        int rivalLeader = getLeaderInRegion(*regions.begin(), name);
        if(rivalLeader != -1) {
            Logger::getLogger()->debug("Internal Conflict triggered!");
            //Initiate internal conflict!
            map<uint32_t, uint32_t> combatants;
            combatants[obj->getID()] = *regions.begin();
            combatants[rivalLeader] = *regions.begin();
            TaeTurn* turn = (TaeTurn*) Game::getGame()->getTurnProcess();
            turn->queueCombatTurn(true, combatants);
        }
        starSysData->setRegion(*(regions.begin()));
        stringstream out;
        out << starSysData->getRegion();
        Logger::getLogger()->debug(string("System " + newStarSys->getName() + " added to region " + out.str()).c_str());
    } else {
        //This code should never be reached! It should've been caught already!
        Logger::getLogger()->warning(string("A leader is attempting to join multiple regions at System: " + newStarSys->getName() + ".  This should've been caught earlier!").c_str());
        return false;
    }

    //Remove old parent from region
    IGObject* parent = obm->getObject(obj->getParent());
    if(parent->getType() == obtm->getObjectTypeByName("Star System")) {
        StarSystem* parentData = (StarSystem*) (parent->getObjectBehaviour());
        parentData->setRegion(0);
    }
    obj->removeFromParent();
    obj->addToParent(starSys->getObjectId());
    fleetData->setPosition(((StarSystem*)(newStarSys->getObjectBehaviour()))->getPosition());

    obm->doneWithObject(newStarSys->getID());
   
 
    // post completion message
    Message * msg = new Message();
    msg->setSubject("Move fleet order complete");
    msg->setBody(string("You're fleet, \"" + obj->getName() + "\" has moved to ")
            + newStarSys->getName() + ".");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, starSys->getObjectId());
    msg->addReference(rst_Object, obj->getID());

    player->postToBoard(msg);

    return true;
}

bool Move::isBorderingScienceColony(StarSystem* system) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();
    Vector3d pos = system->getPosition();

    //east-west neighbors
    for(int i = -1; i < 2; i+=2) {
        set<uint32_t> ids = obm->getObjectsByPos(pos+Vector3d(80000*i,0,0), 1);
        for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
            IGObject *tempObj = obm->getObject(*j);
            if(tempObj->getType() == obtm->getObjectTypeByName("Planet")) {
                Planet* p = (Planet*)(tempObj->getObjectBehaviour());
                if(p->getResource(5) > 1) {
                    return true;
                }
            }
        }
    }
    //north-south neighbors
    for(int i = -1; i < 2; i+=2) {
        set<uint32_t> ids = obm->getObjectsByPos(pos+Vector3d(0,80000*i,0), 1);
        for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
            IGObject *tempObj = obm->getObject(*j);
            if(tempObj->getType() == obtm->getObjectTypeByName("Planet")) {
                Planet* p = (Planet*)(tempObj->getObjectBehaviour());
                if(p->getResource(5) > 1) {
                    return true;
                }
            }
        }
    }
    return false;
}
