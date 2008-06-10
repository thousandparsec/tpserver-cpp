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

#include "ownedobject.h"
#include "fleet.h"
#include "planet.h"

#include "colonize.h"

using std::set;
using std::string;

Colonize::Colonize() : Order() {
    name = "Colonize";
    description = "Colonize a given system";

    starSys = new ObjectOrderParameter();
    starSys->setName("Star System");
    starSys->setDescription("The star system to Colonize");
    addOrderParameter(starSys);
}

Colonize::~Colonize() {

}

Order* Colonize::clone() const {
    Colonize* o = new Colonize();
    o->type = type;
    return o;
}

void Colonize::createFrame(Frame *f, int pos) {
    Order::createFrame(f, pos);
}

Result Colonize::inputFrame(Frame *f, uint32_t playerid) {
    Result r = Order::inputFrame(f, playerid);
    if(!r) return r;

    turns = 0;

    Game *game = Game::getGame();
    ObjectManager *obm = game->getObjectManager();
    ObjectTypeManager *obtm = game->getObjectTypeManager();


    IGObject *fleet = obm->getObject(game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());

    Fleet* fleetData = (Fleet*)(fleet->getObjectBehaviour());
    assert(fleetData != NULL);

    IGObject *starSysObj = obm->getObject(starSys->getObjectId());

    // if they chose a planet, set to the owning star sys
    if(starSysObj->getType() == obtm->getObjectTypeByName("Planet"))
    {
        starSys->setObjectId(starSysObj->getParent());
        Logger::getLogger()->debug("Player trying to colonize to planet, setting to planet's star sys");
    }
    // if they're just crazy, reset to current position
    else if(starSysObj->getType() != obtm->getObjectTypeByName("Star System"))
    {
        starSys->setObjectId(fleet->getParent());
        Logger::getLogger()->debug("Player made illogical colonize order, resetting colonize to current pos");
    }

    //TODO: Check to see if it is a legal system for this fleet (not colonized, 
    //      occupied, or destroyed)

    obm->doneWithObject(fleet->getID());

    return Success();
}

bool Colonize::doOrder(IGObject * obj) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();
    Fleet* fleetData = (Fleet*)(obj->getObjectBehaviour());

    //Find the star system's planet
    IGObject *newStarSys = obm->getObject(starSys->getObjectId());
    set<uint32_t> children = newStarSys->getContainedObjects();
    uint32_t pid = -1;
    for(set<uint32_t>::iterator i=children.begin(); i != children.end(); i++) {
        if(obm->getObject(*i)->getType() == obtm->getObjectTypeByName("Planet")) {
            pid = *i;
        }
    }
    if(pid == -1) {
        Logger::getLogger()->debug("Colonize Order: No planet found in target star system");
        return false;
    }

    //Add resource to planet
    Planet* planet = (Planet*)(obm->getObject(pid)->getObjectBehaviour());
    planet->addResource(4, 1);
    obm->doneWithObject(newStarSys->getID());
   
 
    // post completion message
    Player* player = Game::getGame()->getPlayerManager()->getPlayer(fleetData->getOwner());
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
