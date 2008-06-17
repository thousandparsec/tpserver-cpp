/*  FleetOrder order, the base class for fleet related orders in TaE
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
#include <tpserver/logging.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>

#include "ownedobject.h"
#include "starsystem.h"
#include "fleet.h"
#include "planet.h"

#include "fleetorder.h"

using std::set;
using std::string;
using std::list;

FleetOrder::FleetOrder() : Order() {
    starSys = new ObjectOrderParameter();
    starSys->setName("Star System");
    starSys->setDescription("The star system associated with this order");
    addOrderParameter(starSys);
}

FleetOrder::~FleetOrder() {

}

void FleetOrder::createFrame(Frame *f, int pos) {
    Order::createFrame(f, pos);
}

Result FleetOrder::inputFrame(Frame *f, uint32_t playerid) {
    Result r = Order::inputFrame(f, playerid);
    if(!r) return r;

    turns = 0;

    Game *game = Game::getGame();
    ObjectManager *obm = game->getObjectManager();
    ObjectTypeManager *obtm = game->getObjectTypeManager();

    IGObject *starSysObj = obm->getObject(starSys->getObjectId());

    // if they chose a planet, set to the owning star sys
    if(starSysObj->getType() == obtm->getObjectTypeByName("Planet"))
    {
        starSys->setObjectId(0);
        Logger::getLogger()->debug("Player trying to issue a fleet order to planet, setting to planet's star sys");
    }
    // if they're just crazy, reset to current position
    else if(starSysObj->getType() != obtm->getObjectTypeByName("Star System"))
    {
        starSys->setObjectId(0);
        Logger::getLogger()->debug("Player made illogical fleet order, resetting to current pos");
    }
    //Check to make sure that no other order is targeting this system       
    else {
        OrderManager* ordermanager = game->getOrderManager();
        set<uint32_t> objects = obm->getAllIds();
        //Check all the objects
        for(set<uint32_t>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
            IGObject * ob = obm->getObject(*itcurr);
            //Find the Fleets
            if(ob->getType() == obtm->getObjectTypeByName("Fleet")){
                OwnedObject* owned = (OwnedObject*) ob->getObjectBehaviour();
                //Make sure they're owned by this player
                if(owned->getOwner() == playerid) {
                    //Check to see if they have been given an order
                    OrderQueueObjectParam* oqop = (OrderQueueObjectParam*)(ob->getParameterByType(obpT_Order_Queue));
                    if(oqop != NULL){
                        OrderQueue* orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
                        if(orderqueue != NULL){
                            Order * currOrder = orderqueue->getFirstOrder();
                            if(currOrder != NULL){
                                //See if the order is to this star system
                                list<OrderParameter*> paramList = currOrder->getParameters();
                                list<OrderParameter*>::iterator i = paramList.begin();
                                OrderParameter* param = *i;
                                if(param->getName().compare("Star System") == 0) {
                                    if(((ObjectOrderParameter*)param)->getObjectId() == starSys->getObjectId()) {
                                        starSys->setObjectId(0);
                                        Logger::getLogger()->debug("Player trying to initiate a fleet order on a system already targeted for an action this turn");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return Success();
}
