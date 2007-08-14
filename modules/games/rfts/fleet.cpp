/*  fleet
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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
#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/object.h>
#include <tpserver/objectdata.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/frame.h>
#include <tpserver/designstore.h>
#include <tpserver/design.h>

#include <tpserver/integerobjectparam.h>
#include <tpserver/refquantitylistobjectparam.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/velocity3dobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/objectparametergroup.h>

#include <tpserver/refsys.h>

#include "nop.h"
#include "move.h"
#include "containertypes.h"
#include "rfts.h"

#include "fleet.h"

namespace RFTS_ {

using std::map;
using std::set;
using std::pair;
using std::string;

Fleet::Fleet() {
   nametype = "Fleet";
   typedesc = "A fleet of ships";

   ObjectParameterGroup *group = paramgroups.front();

   velocity = new Velocity3dObjectParam();
   velocity->setName("Velocity");
   velocity->setDescription("The velocity of the fleet");
   group->addParameter(velocity);

   group = new ObjectParameterGroup();
   group->setGroupId(2);
   group->setName("Ownership");
   group->setDescription("The ownership of this object");
   player = new ReferenceObjectParam();
   player->setName("Owner");
   player->setDescription("The owner of this object");
   player->setReferenceType(rst_Player);
   group->addParameter(player);
   paramgroups.push_back(group);

   group = new ObjectParameterGroup();
   group->setGroupId(3);
   group->setName("Orders");
   group->setDescription("The order queue for this object");
   orders = new OrderQueueObjectParam();
   orders->setName("Order queue");
   orders->setDescription("The order queue for this object");
   group->addParameter(orders);
   paramgroups.push_back(group);

   group = new ObjectParameterGroup();
   group->setGroupId(4);
   group->setName("Ships");
   group->setDescription("The fleet's ship's information");
   shipList = new RefQuantityListObjectParam();
   shipList->setName("Ship list");
   shipList->setDescription("The list of ships in this fleet");
   group->addParameter(shipList);
   damage = new IntegerObjectParam();
   damage->setName("Damage");
   damage->setDescription("The damage taken by the ships");
   damage->setValue(0);
   group->addParameter(damage);
   paramgroups.push_back(group);  
}

uint32_t Fleet::getOwner() const {
   return player->getReferencedId();
}

void Fleet::setOwner(uint32_t newOwner) {
   player->setReferencedId(newOwner);
}

int Fleet::getDamage() const {
   return damage->getValue();
}

void Fleet::setDamage(int newDmg) {
   damage->setValue(newDmg);
}

void Fleet::takeDamage(int dmg) {
   damage->setValue( damage->getValue() - dmg );
}

void Fleet::setVelocity(const Vector3d& nv) {
   velocity->setVelocity(nv);
}

void Fleet::setDefaultOrderTypes() {
   OrderManager *om = Game::getGame()->getOrderManager();
   std::set<uint32_t> allowedlist;
   allowedlist.insert(om->getOrderTypeByName("No Operation"));
   allowedlist.insert(om->getOrderTypeByName("Move"));
   orders->setAllowedOrders(allowedlist);
}


void Fleet::addShips(uint32_t type, uint32_t number) {
   map<pair<int32_t, uint32_t>, uint32_t> ships = shipList->getRefQuantityList();
   ships[pair<int32_t, uint32_t>(rst_Design, type)] += number;
   shipList->setRefQuantityList(ships);

   DesignStore *ds = Game::getGame()->getDesignStore();
   OrderManager* om = Game::getGame()->getOrderManager();
   Design *design = ds->getDesign(type);
   
   if(design->getPropertyValue(ds->getPropertyByName("Colonise")) == 1){
      std::set<uint32_t> allowed = orders->getAllowedOrders();
      allowed.insert(om->getOrderTypeByName("Colonise"));
      orders->setAllowedOrders(allowed);
   }
   if(design->getPropertyValue(ds->getPropertyByName("Speed")) > speed)
      speed = design->getPropertyValue(ds->getPropertyByName("Speed"));
   if(design->getPropertyValue(ds->getPropertyByName("Armour")) > armour)
      armour = design->getPropertyValue(ds->getPropertyByName("Armour"));
   if(design->getPropertyValue(ds->getPropertyByName("Attack")) > attack)
      attack = design->getPropertyValue(ds->getPropertyByName("Attack"));
      
   touchModTime();
}

bool Fleet::removeShips(int type, uint32_t number){
  map<pair<int32_t, uint32_t>, uint32_t> ships = shipList->getRefQuantityList();
  if(ships[pair<int32_t, uint32_t>(rst_Design, type)] >= number){
    ships[pair<int32_t, uint32_t>(rst_Design, type)] -= number;
    if(ships[pair<int32_t, uint32_t>(rst_Design, type)] == 0){
      ships.erase(pair<int32_t, uint32_t>(rst_Design, type));
    }
    shipList->setRefQuantityList(ships);

   recalcStats();

    touchModTime();
    return true;
  }
  return false;
}

int Fleet::numShips(int type){
  map<pair<int32_t, uint32_t>, uint32_t> ships = shipList->getRefQuantityList();
  return ships[pair<int32_t, uint32_t>(rst_Design, type)];
}

map<int,int> Fleet::getShips() const{
  map<int,int> ships;
  map<pair<int32_t, uint32_t>, uint32_t> shipsref = shipList->getRefQuantityList();
  for(map<pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = shipsref.begin();
      itcurr != shipsref.end(); ++itcurr){
    ships[itcurr->first.second] = itcurr->second;
  }
  return ships;
}

int Fleet::totalShips() const{
  int num = 0;
  map<pair<int32_t, uint32_t>, uint32_t> ships = shipList->getRefQuantityList();
  for(map<pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
    num += itcurr->second;
  }
  return num;
}


const double Fleet::getSpeed() const {
   return speed;
}

const double Fleet::getAttack() const {
   return attack;
}

const double Fleet::getArmour() const {
   return armour;
}

void Fleet::recalcStats() {
   map<pair<int32_t, uint32_t>, uint32_t> shipsref = shipList->getRefQuantityList();

   DesignStore *ds = Game::getGame()->getDesignStore();
   speed = armour = attack = 0;
   bool colonise = false;

   for(map<pair<int32_t, uint32_t>, uint32_t>::const_iterator i = shipsref.begin();
         i != shipsref.end(); ++i) //ships[i->first.second] = i->second;
   {
      Design *d = ds->getDesign(i->first.second);
      
      if(d->getPropertyValue(ds->getPropertyByName("Speed")) > speed)
         speed = d->getPropertyValue(ds->getPropertyByName("Speed"));
      if(d->getPropertyValue(ds->getPropertyByName("Attack")) > attack)
         attack = d->getPropertyValue(ds->getPropertyByName("Attack"));
      if(d->getPropertyValue(ds->getPropertyByName("Armour")) > armour)
         armour = d->getPropertyValue(ds->getPropertyByName("Armour"));

      if(d->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.)
         colonise = true;
   }

   OrderManager* om = Game::getGame()->getOrderManager();
   set<uint32_t> allowed = orders->getAllowedOrders();
   
   if(colonise)
      allowed.insert(om->getOrderTypeByName("Colonise"));
   else
      allowed.erase(om->getOrderTypeByName("Colonise"));
      
   orders->setAllowedOrders(allowed);
}

void Fleet::packExtraData(Frame *frame) {
   ObjectData::packExtraData(frame);
  
   frame->packInt((player->getReferencedId() == 0) ? 0xffffffff : player->getReferencedId());
   
   map<pair<int32_t, uint32_t>, uint32_t> ships = shipList->getRefQuantityList();
   frame->packInt(ships.size());
   for(map<pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin();
       itcurr != ships.end(); ++itcurr)
   {
         frame->packInt(itcurr->first.second);
         frame->packInt(itcurr->second);

   }
   
   frame->packInt(damage->getValue());
}

void Fleet::doOnceATurn(IGObject *obj) {

   // TODO
   // if in a star sys with an opposing fleet
   //    then set attack order
   //    else remove attack order
   // else (no opposing fleet here)
   //    then if opposing planet is present
   //       then add bombard order
   //       else remove bombard order
   //    add colonise order
}

int Fleet::getContainerType() {
   return ContainerTypes_::Fleet;
}

ObjectData* Fleet::clone() const {
   return new Fleet();
}



IGObject* createEmptyFleet(Player* player, IGObject* starSys, const std::string& name)
{
   Game *game = Game::getGame();
   IGObject *fleet = game->getObjectManager()->createNewObject();
      
   fleet->setType(game->getObjectDataManager()->getObjectTypeByName("Fleet"));
   if(name[0] == '\0' || name[0] == ' ')
      fleet->setName(player->getName() + "'s New Fleet");
   else
      fleet->setName(name);
      
   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectData());
   fleetData->setSize(2);
   fleetData->setOwner(player->getID());

   // Place the fleet in orbit around the given star
   fleetData->setPosition( dynamic_cast<StaticObject*>(starSys->getObjectData())->getPosition());
   fleetData->setVelocity( Vector3d(0LL, 0ll, 0ll));
   
   OrderQueue *fleetoq = new OrderQueue();
   fleetoq->setObjectId(fleet->getID());
   fleetoq->addOwner(player->getID());
   game->getOrderManager()->addOrderQueue(fleetoq);
   OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(
                                 fleetData->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId(fleetoq->getQueueId());
   fleetData->setDefaultOrderTypes();

   fleet->addToParent(starSys->getID());

   return fleet;
}

pair<IGObject*,uint32_t> createFleet(Player *player, IGObject* starSys, const string& name,
                        const map<uint32_t, uint32_t>& ships, uint32_t availableRP) {
   IGObject* fleet = createEmptyFleet(player, starSys, name);
   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectData());

   uint32_t usedRP = 0;

   for(map<uint32_t, uint32_t>::const_iterator i = ships.begin();
       i != ships.end() &&
       (usedRP += Rfts::getProductionInfo().getResourceCost(
                  Game::getGame()->getDesignStore()->getDesign(i->first)->getName())) < availableRP; ++i)
   {
      fleetData->addShips(i->first, i->second);
   }

   return pair<IGObject*, uint32_t>(fleet, usedRP);
}


}
