/*  FleetOrder class.  This class is used as a base class for the normal
 *  fleet based orders.  It encorporates many functions and data which
 *  each of these orders need.
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
#include <tpserver/orderparameters.h>

#include "ownedobject.h"
#include "starsystem.h"
#include "fleet.h"
#include "planet.h"

#include "fleetorder.h"

using std::set;
using std::string;
using std::list;

FleetOrder::FleetOrder() : Order() {
    starSys = (ObjectOrderParameter*) addOrderParameter( new ObjectOrderParameter("Star System", "The star system associated with this order") );
}

FleetOrder::~FleetOrder() {

}

void FleetOrder::createFrame(OutputFrame *f, int pos) {
    Order::createFrame(f, pos);
}

//Used to check to see if the input is valid
void FleetOrder::inputFrame(InputFrame *f, uint32_t playerid) {
    Order::inputFrame(f, playerid);

    turns = 0;
    int numOrders = 0;

    Game *game = Game::getGame();
    ObjectManager *obm = game->getObjectManager();
    ObjectTypeManager *obtm = game->getObjectTypeManager();

    IGObject::Ptr starSysObj = obm->getObject(starSys->getObjectId());

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
            IGObject::Ptr ob = obm->getObject(*itcurr);
            //Find the Fleets
            if(ob->getType() == obtm->getObjectTypeByName("Fleet")){
                OwnedObject* owned = (OwnedObject*) ob->getObjectBehaviour();
                //Make sure they're owned by this player
                if(owned->getOwner() == playerid) {
                    //Check to see if they have been given an order
                    OrderQueueObjectParam* oqop = (OrderQueueObjectParam*)(ob->getParameterByType(obpT_Order_Queue));
                    if(oqop != NULL){
                      OrderQueue::Ptr orderqueue = ordermanager->getOrderQueue(oqop->getQueueId());
                        if(orderqueue != NULL){
                            Order * currOrder = orderqueue->getFirstOrder();
                            if(currOrder != NULL){
                                numOrders++;
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

    //Check to see if the maximum number of orders has already been issued
    //TODO: Change this so they cant even create another order once 2 have been created
    if(numOrders > 1) {
        starSys->setObjectId(0);
        Logger::getLogger()->debug("Player has already issued 2 or more orders this turn.");
    }
}

//Returns the regions bordering the fleet
set<uint32_t> FleetOrder::getBorderingRegions() {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();
    IGObject::Ptr newStarSys = obm->getObject(starSys->getObjectId());
    StarSystem* starSysData = (StarSystem*)(newStarSys->getObjectBehaviour());
    set<uint32_t> regions;
    Vector3d pos = starSysData->getPosition();
    //Check the neighbors:
    //east-west neighbors
    for(int i = -1; i < 2; i+=2) {
        set<uint32_t> ids = obm->getObjectsByPos(pos+Vector3d(80000*i,0,0), 1);
        for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
            IGObject::Ptr tempObj = obm->getObject(*j);
            if(tempObj->getType() == obtm->getObjectTypeByName("Star System")) {
                uint32_t r = ((StarSystem*)(tempObj->getObjectBehaviour()))->getRegion();
                if(r != 0 && regions.count(r) == 0) {
                    regions.insert(r);
                }
            }
        }
    }
    //north-south neighbors
    for(int i = -1; i < 2; i+=2) {
        set<uint32_t> ids = obm->getObjectsByPos(pos+Vector3d(0,80000*i,0), 1);
        for(set<uint32_t>::iterator j=ids.begin(); j != ids.end(); j++) {
            IGObject::Ptr tempObj = obm->getObject(*j);
            if(tempObj->getType() == obtm->getObjectTypeByName("Star System")) {
                uint32_t r = ((StarSystem*)(tempObj->getObjectBehaviour()))->getRegion();
                if(r != 0 && regions.count(r) == 0) {
                    regions.insert(r);
                }
            }
        }
    }
    return regions;
}

//Get the leader of type leaderType in the specified region
int FleetOrder::getLeaderInRegion(uint32_t region, string leaderType) {
    ObjectManager* obm = Game::getGame()->getObjectManager();
    ObjectTypeManager* obtm = Game::getGame()->getObjectTypeManager();
    
    //Check all the objects for the leader
    set<uint32_t> objects = obm->getAllIds();
    for(set<uint32_t>::iterator itcurr = objects.begin(); itcurr != objects.end(); ++itcurr) {
        IGObject::Ptr ob = obm->getObject(*itcurr);
        //Find the Fleets
        if(ob->getType() == obtm->getObjectTypeByName("Fleet")){
            Logger::getLogger()->debug("Found a fleet");
            Fleet* f = (Fleet*) (ob->getObjectBehaviour());
            //Check the ship
            uint32_t ship = f->getShips().begin()->first;
            if(Game::getGame()->getDesignStore()->getDesign(ship)->getName().compare(leaderType) == 0) {
                IGObject::Ptr parent = obm->getObject(ob->getParent());
                Logger::getLogger()->debug(parent->getName().c_str());
                //Make sure the fleet is orbiting a star system
                if(parent->getType() == obtm->getObjectTypeByName("Star System")) {
                    StarSystem* parentData = (StarSystem*) (parent->getObjectBehaviour());
                    //Check the region
                    if(parentData->getRegion() == region) {
                        return *itcurr;
                    }
                }
            }
        }
    }
    
    return -1;
}
