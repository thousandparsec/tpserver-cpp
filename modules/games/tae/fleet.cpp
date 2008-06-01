/*  Fleet object
 *
 *  Copyright (C) 2008 Dustin White and the Thousand Parsec Project
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <math.h>

#include <tpserver/frame.h>
#include <tpserver/order.h>
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/refsys.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/refquantitylistobjectparam.h>
#include "planet.h"

#include "fleet.h"

FleetType::FleetType():OwnedObjectType(){
    //Set parameters of the fleet
    ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
    group->setName("Ships");
    group->setDescription("The information about ships in this fleet");
    group->addParameter(obpT_Reference_Quantity_List, "Ship List", "The list of ships");
    addParameterGroupDesc(group);

    nametype = "Fleet";
    typedesc = "Fleet of ships";
}

FleetType::~FleetType(){
}

ObjectBehaviour* FleetType::createObjectBehaviour() const{
    return new Fleet();
}

Fleet::Fleet():OwnedObject(){
}

Fleet::~Fleet(){
}

void Fleet::setDefaultOrderTypes(){
    //TODO: Write this function... example below
    /*
    OrderManager * om = Game::getGame()->getOrderManager();
    std::set<uint32_t> allowedlist;
    allowedlist.insert(om->getOrderTypeByName("No Operation"));
    allowedlist.insert(om->getOrderTypeByName("Move"));
    allowedlist.insert(om->getOrderTypeByName("Split Fleet"));
    allowedlist.insert(om->getOrderTypeByName("Merge Fleet"));
    ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(allowedlist);
    */
}

void Fleet::addShips(uint32_t type, uint32_t number){
    //Get list of ships
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    //Add the new ships to list
    ships[std::pair<int32_t, uint32_t>(rst_Design, type)] += number;
    //Set list of ships
    ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->setRefQuantityList(ships);
    obj->touchModTime();
}

bool Fleet::removeShips(uint32_t type, uint32_t number){
    //Get list of ships
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    //Check to make sure the request is valid, then delete the ships
    if(ships[std::pair<int32_t, uint32_t>(rst_Design, type)] >= number){
        ships[std::pair<int32_t, uint32_t>(rst_Design, type)] -= number;
        if(ships[std::pair<int32_t, uint32_t>(rst_Design, type)] == 0){
            ships.erase(std::pair<int32_t, uint32_t>(rst_Design, type));
        }
        ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->setRefQuantityList(ships);
        obj->touchModTime();
        return true;
    }
    return false;
}

uint32_t Fleet::numShips(uint32_t type){
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    return ships[std::pair<int32_t, uint32_t>(rst_Design, type)];
}

std::map<uint32_t, uint32_t> Fleet::getShips() const{
    std::map<uint32_t, uint32_t> ships;
    std::map<std::pair<int32_t, uint32_t>, uint32_t> shipsref = (( RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = shipsref.begin();
            itcurr != shipsref.end(); ++itcurr){
        ships[itcurr->first.second] = itcurr->second;
    }
    return ships;
}

uint32_t Fleet::totalShips() const{
    uint32_t num = 0;
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = ships.begin();
            itcurr != ships.end(); ++itcurr){
        num += itcurr->second;
    }
    return num;
}

void Fleet::packExtraData(Frame * frame){
    OwnedObject::packExtraData(frame);

    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    frame->packInt(ships.size());
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
        //if(itcurr->second > 0){
        frame->packInt(itcurr->first.second);
        frame->packInt(itcurr->second);
        //}
    }

    frame->packInt(((IntegerObjectParam*)(obj->getParameter(4,2)))->getValue());

}

void Fleet::doOnceATurn(){
}

int Fleet::getContainerType(){
    return 0;
}

void Fleet::setupObject(){
    OwnedObject::setupObject();

    setSize(2);
}

