/*  Move order
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

#include "move.h"

using std::set;
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

    IGObject* newStarSys = obm->getObject(starSys->getObjectId());
    if(newStarSys->getType() != obtm->getObjectTypeByName("Star System")) {
        Logger::getLogger()->debug("Trying to move to an object which is not a star system");
        return false;
    }
    StarSystem* starSysData = (StarSystem*)(newStarSys->getObjectBehaviour());

    //Set new parent's region
    set<uint32_t> regions = getBorderingRegions();
    //If it does not border any regions, the add it to a new one
    if(regions.size() == 0) {
        starSysData->setRegion(starSys->getObjectId());
        stringstream out;
        out << starSysData->getRegion();
        Logger::getLogger()->debug(string("System " + newStarSys->getName() + " added to region " + out.str()).c_str());
    } else if(regions.size() == 1) {
        //TODO: Check for another leader of the fleet type in this region.
        //      If there is one, initiate INTERNAL conflict!
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
    Player* player = Game::getGame()->getPlayerManager()->getPlayer(fleetData->getOwner());
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
