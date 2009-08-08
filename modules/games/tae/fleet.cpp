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

#include <tpserver/logging.h>
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

using std::string;

FleetType::FleetType():OwnedObjectType( "Fleet", "Fleet of ships" ) {
    Logger::getLogger()->debug("Enter: Fleet Constructor");
    //Set parameters of the fleet
    ObjectParameterGroupDesc* group = new ObjectParameterGroupDesc();
    group->setName("Ships");
    group->setDescription("The information about ships in this fleet");
    group->addParameter(obpT_Reference_Quantity_List, "Ship List", "The list of ships");
    group->addParameter(obpT_Integer, "Damage", "The damage done to the ships");
    addParameterGroupDesc(group);

    Logger::getLogger()->debug("Exit: Fleet Constructor");
}

FleetType::~FleetType(){
}

ObjectBehaviour* FleetType::createObjectBehaviour() const{
    return new Fleet();
}

Fleet::Fleet():OwnedObject(){
    combat = false;
    combatant = false;
}

Fleet::~Fleet(){
}

void Fleet::setDefaultOrderTypes(){
}

//Add an allowed NORMAL order
void Fleet::addAllowedOrder(string order) {
    Logger::getLogger()->debug("Enter: Fleet::addAllowedOrder");
    OrderManager * om = Game::getGame()->getOrderManager();
    normalOrders.insert(om->getOrderTypeByName(order));
    if(!combat) {
        ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(normalOrders);
    }
    Logger::getLogger()->debug("Exit: Fleet::addAllowedOrder");
}

//Add an allowed COMBAT order
void Fleet::addAllowedCombatOrder(string order) {
    OrderManager * om = Game::getGame()->getOrderManager();
    combatOrders.insert(om->getOrderTypeByName(order));
    if(combat) {
        ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(combatOrders);
    }
    Logger::getLogger()->debug("Exit: Fleet::addAllowedCombatOrder");
}

//Toggles whether the current turn is a combat turn or not
//NOTE: if it IS a combat turn, but this fleet is NOT a combatant,
//      then it will have NO orders, otherwise (if it is a comatant)
//      it will use its combat orders.
void Fleet::toggleCombat() {
    combat = !combat;
    if(combat) {
        if(combatant) {
            //If current turn is combat turn and this fleet is a combatant then enable
            //its combat orders
            ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(combatOrders);
        } else {
            //if it is not a combatant, then it gets no orders
            std::set<uint32_t> temp;
            ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(temp);
        }
    } else {
        //if the current turn is not a combat turn then use its normal orders
        ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(normalOrders);
        combatant = false;
    } 
}

//Sets whether the fleet is a combatant in the combat turn
void Fleet::setCombatant(bool com) {
    combatant = com;
    if(combat && com) {
        ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(combatOrders);
    }
}

//Gets whether or not the current fleet is a combatant in the cunbat turn
bool Fleet::isCombatant() {
    return combatant;
}

//Add ships to the fleet
void Fleet::addShips(uint32_t type, uint32_t number){
    Logger::getLogger()->debug("Enter: Fleet::addships");
    //Get list of ships
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    //Add the new ships to list
    ships[std::pair<int32_t, uint32_t>(rst_Design, type)] += number;
    //Set list of ships
    ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->setRefQuantityList(ships);
    obj->touchModTime();
    Logger::getLogger()->debug("Exit: Fleet::addShips");
}

//Remove ships from the fleet
bool Fleet::removeShips(uint32_t type, uint32_t number){
    Logger::getLogger()->debug("Enter: Fleet::removeShips");
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
        Logger::getLogger()->debug("Exit: Fleet::removeShips");
        return true;
    }
    Logger::getLogger()->debug("Exit: Fleet::removeShips");
    return false;
}

//Get the number of ships of the specified type in the fleet
uint32_t Fleet::numShips(uint32_t type){
    Logger::getLogger()->debug("Enter: Fleet::numShips");
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    Logger::getLogger()->debug("Exit: Fleet::numShips");
    return ships[std::pair<int32_t, uint32_t>(rst_Design, type)];
}

//Get a map of how many ships of each type are in the fleet.
std::map<uint32_t, uint32_t> Fleet::getShips() const{
    Logger::getLogger()->debug("Enter: Fleet::getShips");
    std::map<uint32_t, uint32_t> ships;
    std::map<std::pair<int32_t, uint32_t>, uint32_t> shipsref = (( RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    Logger::getLogger()->debug("got ship list");
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = shipsref.begin();
            itcurr != shipsref.end(); ++itcurr){    
        Logger::getLogger()->debug("step");
        ships[itcurr->first.second] = itcurr->second;
    }
    Logger::getLogger()->debug("Exit: Fleet::getShips");
    return ships;
}

//Get the total number of ships in this fleet
uint32_t Fleet::totalShips() const{
    Logger::getLogger()->debug("Enter: Fleet::totalShips");
    uint32_t num = 0;
    std::map<std::pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
    for(std::map<std::pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = ships.begin();
            itcurr != ships.end(); ++itcurr){
        num += itcurr->second;
    }
    Logger::getLogger()->debug("Exit: Fleet::totalShips");
    return num;
}

//Get damage done to ship (NOT CURRENTLY USED)
uint32_t Fleet::getDamage() const{
    return ((IntegerObjectParam*)(obj->getParameter(4,2)))->getValue();
}

//Set damage done to ship (NOT CURRENTLY USED)
void Fleet::setDamage(uint32_t nd){
    ((IntegerObjectParam*)(obj->getParameter(4,2)))->setValue(nd);
    obj->touchModTime();
}

void Fleet::packExtraData(Frame * frame){
    Logger::getLogger()->debug("Enter: Fleet::packExtraData");
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
    Logger::getLogger()->debug("Exit: Fleet::packExtraData");

}

void Fleet::doOnceATurn(){
}

int Fleet::getContainerType(){
    return 0;
}

void Fleet::setupObject(){
    Logger::getLogger()->debug("Enter: Fleet::setupObject");
    OwnedObject::setupObject();

    setSize(2);
    Logger::getLogger()->debug("Exit: Fleet::setupObject");
}

