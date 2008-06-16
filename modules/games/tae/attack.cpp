/*  Attack order
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
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/orderparameter.h>
#include <tpserver/logging.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>

#include "ownedobject.h"
#include "fleet.h"
#include "planet.h"
#include "starsystem.h"

#include "attack.h"

using std::set;
using std::list;
using std::string;

Attack::Attack() : Order() {
    name = "Attack";
    description = "Attack a given system";

    starSys = new ObjectOrderParameter();
    starSys->setName("Star System");
    starSys->setDescription("The star system to Attack");
    addOrderParameter(starSys);
}

Attack::~Attack() {

}

Order* Attack::clone() const {
    Attack* o = new Attack();
    o->type = type;
    return o;
}

void Attack::createFrame(Frame *f, int pos) {
    Order::createFrame(f, pos);
}

Result Attack::inputFrame(Frame *f, uint32_t playerid) {
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
    StarSystem* starSysData = (StarSystem*) starSysObj->getObjectBehaviour();

    // if they chose a planet, set to the owning star sys
    if(starSysObj->getType() == obtm->getObjectTypeByName("Planet"))
    {
        starSys->setObjectId(starSysObj->getParent());
        Logger::getLogger()->debug("Player trying to destroy to planet, setting to planet's star sys");
    }
    // if they're just crazy, reset to current position
    else if(starSysObj->getType() != obtm->getObjectTypeByName("Star System"))
    {
        starSys->setObjectId(fleet->getParent());
        Logger::getLogger()->debug("Player made illogical destroy order, resetting colonize to current pos");
    }
    //Check to see if star system is already destroyed
    else if(starSysData->isDestroyed()) {
        starSys->setObjectId(fleet->getParent());
        Logger::getLogger()->debug("Player trying to destroy a system that is already destroyed");
    }
    //Check to make sure that no other order is targeting this system 
    else {
        OrderManager* ordermanager = game->getOrderManager();
        set<uint32_t> objects = obm->getAllIds();
        for(set<uint32_t>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
            IGObject * ob = obm->getObject(*itcurr);
            if(ob->getType() == obtm->getObjectTypeByName("Fleet")){
                OwnedObject* owned = (OwnedObject*) ob->getObjectBehaviour();
                if(owned->getOwner() == playerid) {
                    OrderQueueObjectParam* oqop = (OrderQueueObjectParam*)(ob->getParameterByType(obpT_Order_Queue));
                    if(oqop != NULL){
                        OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
                        if(orderqueue != NULL){
                            Order * currOrder = orderqueue->getFirstOrder();
                            if(currOrder != NULL){
                                list<OrderParameter*> paramList = currOrder->getParameters();
                                list<OrderParameter*>::iterator i = paramList.begin();
                                OrderParameter* param = *i;
                                if(param->getName().compare("Star System") == 0) {
                                    if(((ObjectOrderParameter*)param)->getObjectId() == starSys->getObjectId()) {
                                        starSys->setObjectId(fleet->getParent());
                                        Logger::getLogger()->debug("Player trying to destroy a system already targeted for an action this turn");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    obm->doneWithObject(fleet->getID());

    return Success();
}

bool Attack::doOrder(IGObject * obj) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();
    Fleet* fleetData = (Fleet*)(obj->getObjectBehaviour());

    //Find the star system's planet
    IGObject *newStarSys = obm->getObject(starSys->getObjectId());
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
        Logger::getLogger()->debug("Attack Order: No planet found in target star system");
        return false;
    }

    //Add Destroyed resource to planet
    Planet* planet = (Planet*)(obm->getObject(pid)->getObjectBehaviour());
    planet->addResource(2, 1);

    //Set star system as destroyed
    StarSystem* sys = (StarSystem*) newStarSys->getObjectBehaviour();    
    sys->setDestroyed(true);

    obm->doneWithObject(newStarSys->getID());


    // post completion message
    Player* player = Game::getGame()->getPlayerManager()->getPlayer(fleetData->getOwner());
    Message * msg = new Message();
    msg->setSubject("Attack fleet order complete");
    msg->setBody(string("You're fleet, \"" + obj->getName() + "\" has destroyed ")
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
