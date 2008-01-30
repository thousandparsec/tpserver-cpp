/*  fleet
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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
 
#include <tpserver/game.h>
#include <tpserver/ordermanager.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>
#include <tpserver/frame.h>
#include <tpserver/designstore.h>
#include <tpserver/design.h>

#include <tpserver/integerobjectparam.h>
#include <tpserver/refquantitylistobjectparam.h>
#include <tpserver/referenceobjectparam.h>
#include <tpserver/velocity3dobjectparam.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/prng.h>
#include <tpserver/refsys.h>

#include "move.h"
#include "renamefleet.h"
#include "splitfleet.h"
#include "containertypes.h"
#include "rfts.h"
#include "productioninfo.h"
#include "playerinfo.h"
#include "planet.h"

#include "fleet.h"

namespace RFTS_ {

using std::map;
using std::set;
using std::pair;
using std::string;
using std::list;

FleetType::FleetType() : StaticObjectType() {
   nametype = "Fleet";
   typedesc = "A fleet of ships";

   ObjectParameterGroupDesc *group = getParameterGroupDesc(1);
   group->addParameter(obpT_Velocity, "Velocity", "The velocity of the fleet");

   group = new ObjectParameterGroupDesc();
   group->setName("Ownership");
   group->setDescription("The ownership of this object");
   group->addParameter(obpT_Reference, "Owner", "The owner of this object");
   addParameterGroupDesc(group);

   group = new ObjectParameterGroupDesc();
   group->setName("Orders");
   group->setDescription("The order queue for this object");
   group->addParameter(obpT_Order_Queue, "Order queue", "The order queue for this object");
   addParameterGroupDesc(group);
   
   group = new ObjectParameterGroupDesc();
   group->setName("Ships");
   group->setDescription("The fleet's ship's information");
   group->addParameter(obpT_Reference_Quantity_List, "Ship list", "The list of ships in this fleet");
   group->addParameter(obpT_Integer, "Damage", "The damage taken by the ships");
   addParameterGroupDesc(group);
}

FleetType::~FleetType(){
}

ObjectBehaviour* FleetType::createObjectBehaviour() const{
  return new Fleet();
}

Fleet::Fleet() : StaticObject(){
}

uint32_t Fleet::getOwner() const {
   return ((ReferenceObjectParam*)(obj->getParameter(2,1)))->getReferencedId();
}

void Fleet::setOwner(uint32_t newOwner) {
   ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferencedId(newOwner);
}

int Fleet::getDamage() const {
   return ((IntegerObjectParam*)(obj->getParameter(4,2)))->getValue();
}

void Fleet::setDamage(int newDmg) {
   ((IntegerObjectParam*)(obj->getParameter(4,2)))->setValue(newDmg);

   destroyShips(armour == 0 ? 99999 : newDmg*.125f / armour);
}

void Fleet::takeDamage(int dmg) {
   setDamage( ((IntegerObjectParam*)(obj->getParameter(4,2)))->getValue() + dmg );
}

void Fleet::setVelocity(const Vector3d& nv) {
   ((Velocity3dObjectParam*)(obj->getParameter(1,3)))->setVelocity(nv);
}

const double Fleet::getAttack() const {
   return attack;
}

const double Fleet::getSpeed() const {
   return speed;
}

void Fleet::setOrderTypes(bool addColonise, bool addBombard) {
   OrderManager *om = Game::getGame()->getOrderManager();
   std::set<uint32_t> allowedList;
   allowedList.insert(om->getOrderTypeByName("Move"));
   allowedList.insert(om->getOrderTypeByName("Split Fleet"));
   allowedList.insert(om->getOrderTypeByName("Merge Fleet"));
   allowedList.insert(om->getOrderTypeByName("Rename Fleet"));

   if(addColonise)
      allowedList.insert(om->getOrderTypeByName("Colonise"));
      
   if(addBombard)
      allowedList.insert(om->getOrderTypeByName("Bombard"));
   
   ((OrderQueueObjectParam*)(obj->getParameter(3,1)))->setAllowedOrders(allowedList);
   
   obj->touchModTime();
}

void Fleet::addShips(uint32_t type, uint32_t number) {
   if(number == 0)
      return;

   map<pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
   ships[pair<int32_t, uint32_t>(rst_Design, type)] += number;
   ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->setRefQuantityList(ships);

   recalcStats();
      
   obj->touchModTime();
}

bool Fleet::removeShips(int type, uint32_t number){
   map<pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
   if(ships[pair<int32_t, uint32_t>(rst_Design, type)] >= number)
   {
      ships[pair<int32_t, uint32_t>(rst_Design, type)] -= number;
      
      if(ships[pair<int32_t, uint32_t>(rst_Design, type)] == 0)
         ships.erase(pair<int32_t, uint32_t>(rst_Design, type));

      ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->setRefQuantityList(ships);

      recalcStats();

      obj->touchModTime();
      return true;
   }
   return false;
}

int Fleet::numShips(int type){
  map<pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
  return ships[pair<int32_t, uint32_t>(rst_Design, type)];
}

map<int,int> Fleet::getShips() const{
  map<int,int> ships;
  map<pair<int32_t, uint32_t>, uint32_t> shipsref = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
  for(map<pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = shipsref.begin();
      itcurr != shipsref.end(); ++itcurr){
    ships[itcurr->first.second] = itcurr->second;
  }
  return ships;
}

int Fleet::totalShips() const{
  int num = 0;
  map<pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
  for(map<pair<int32_t, uint32_t>, uint32_t>::const_iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
    num += itcurr->second;
  }
  return num;
}

const bool Fleet::isDead() const {
   return totalShips() == 0;
}

void Fleet::recalcStats() {
   map<pair<int32_t, uint32_t>, uint32_t> shipsref = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();

   DesignStore *ds = Game::getGame()->getDesignStore();
   speed = armour = attack = 0;
   
   hasTransports = false;

   for(map<pair<int32_t, uint32_t>, uint32_t>::const_iterator i = shipsref.begin();
         i != shipsref.end(); ++i)
   {
      Design *d = ds->getDesign(i->first.second);
      
      if(speed == 0 || d->getPropertyValue(ds->getPropertyByName("Speed")) < speed)
         speed = d->getPropertyValue(ds->getPropertyByName("Speed"));
      if(d->getPropertyValue(ds->getPropertyByName("Attack")) > attack)
         attack =+ d->getPropertyValue(ds->getPropertyByName("Attack"));
      if(d->getPropertyValue(ds->getPropertyByName("Armour")) > armour)
         armour =+ d->getPropertyValue(ds->getPropertyByName("Armour"));

      if(d->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.)
        hasTransports = true;
   }

   setOrderTypes();
}

void Fleet::packExtraData(Frame *frame) {
   ReferenceObjectParam* player = ((ReferenceObjectParam*)(obj->getParameter(2,1)));
   frame->packInt((player->getReferencedId() == 0) ? 0xffffffff : player->getReferencedId());
   
   map<pair<int32_t, uint32_t>, uint32_t> ships = ((RefQuantityListObjectParam*)(obj->getParameter(4,1)))->getRefQuantityList();
   frame->packInt(ships.size());
   for(map<pair<int32_t, uint32_t>, uint32_t>::iterator itcurr = ships.begin();
       itcurr != ships.end(); ++itcurr)
   {
         frame->packInt(itcurr->first.second);
         frame->packInt(itcurr->second);

   }
   
   frame->packInt(((IntegerObjectParam*)(obj->getParameter(4,2)))->getValue());
}

void Fleet::doOnceATurn() {

   if(obj->getParent() == 0) // fleet is in transit, no updates apply
      return;

   //check for opposing fleets
   list<IGObject*> opposingFleets;
   bool hasOpposingPlanet = setOpposingFleets(obj, opposingFleets);

   if(!opposingFleets.empty())
   {
      if(doCombat(opposingFleets))
      {
         Game::getGame()->getObjectManager()->scheduleRemoveObject(obj->getID());
      }
   }

   setOrderTypes(hasTransports, hasOpposingPlanet && attack != 0);
}

int Fleet::getContainerType() {
   return ContainerTypes_::Fleet;
}

bool Fleet::setOpposingFleets(IGObject* obj, list<IGObject*>& fleets) {

   Game *game = Game::getGame();
   ObjectManager *om = game->getObjectManager();

   set<uint32_t> possibleFleets = om->getObject(obj->getParent())->getContainedObjects();
   uint32_t fleetType = obj->getType();

   bool hasOpposingPlanet = false;
   
   for(set<uint32_t>::iterator i = possibleFleets.begin(); i != possibleFleets.end(); ++i)
   {
      IGObject *objI = om->getObject(*i);
      if(objI->getType() == fleetType)
      {
         Fleet* fleetDataI = dynamic_cast<Fleet*>(objI->getObjectBehaviour());
         assert(fleetDataI);
         if(fleetDataI->getOwner() != this->getOwner() && !fleetDataI->isDead())
            fleets.push_back(objI);
      }
      else
      {
         OwnedObject *owned = dynamic_cast<OwnedObject*>(objI->getObjectBehaviour());
         assert(owned);
         if(owned->getOwner() != 0 && owned->getOwner() != this->getOwner())
            hasOpposingPlanet = true;
      }
   }

   return hasOpposingPlanet;
}

bool Fleet::doCombat(list<IGObject*>& fleets) {
   obj->touchModTime();

   // while !fleets.empty && !dead
      // attack each fleet
      // get attacked by each fleet
      // remove dead fleets

   ObjectManager *om = Game::getGame()->getObjectManager();
   list< list<IGObject*>::iterator > killed;

   while(!fleets.empty() && !isDead() && attack != 0)
   {
      for(list<IGObject*>::iterator i = fleets.begin(); i != fleets.end(); ++i)
      {
         Fleet *fleet = dynamic_cast<Fleet*>((*i)->getObjectBehaviour());
         attackFleet(fleet);
         fleet->attackFleet(this);

         if(fleet->isDead())
         {
            killed.push_back(i);
            om->scheduleRemoveObject((*i)->getID());
         }
      }

      for(list< list<IGObject*>::iterator >::iterator i = killed.begin(); i != killed.end(); ++i)
         fleets.erase(*i);

      killed.clear();
   }

   return isDead();
}

void Fleet::attackFleet(Fleet* opp) {
   opp->takeDamage(static_cast<uint32_t>(attack));
   PlayerInfo::getPlayerInfo(getOwner()).addVictoryPoints(static_cast<uint32_t>(attack));
}

void Fleet::destroyShips(double intensity) {
   Random *rand = Game::getGame()->getRandom();

   map<int,int> ships = getShips();

   map<int,int>::iterator i = ships.begin();

   std::advance(i, rand->getInRange(static_cast<uint32_t>(0), ships.size()));

   int shipType = i->first;
   DesignStore *ds = Game::getGame()->getDesignStore();
   Design *d = ds->getDesign(shipType);

   
   double shipArmour = d->getPropertyValue(ds->getPropertyByName("Armour"));

   if(shipArmour == 0)
   {
      removeShips(shipType, rand->getInRange(static_cast<uint32_t>(1),
                                             static_cast<uint32_t>(numShips(shipType)/2.)));
   }
   else
   {
      shipArmour =  1.f / shipArmour;
   
      int shipNum = static_cast<int>(intensity * rand->getReal2() * shipArmour);

      removeShips(shipType, shipNum);
   }
   
}

void Fleet::setupObject(){
  ((ReferenceObjectParam*)(obj->getParameter(2,1)))->setReferenceType(rst_Player);
}


IGObject* createEmptyFleet(Player* player, IGObject* starSys, const std::string& name) {
   Game *game = Game::getGame();
   IGObject *fleet = game->getObjectManager()->createNewObject();
      
   game->getObjectTypeManager()->setupObject(fleet, game->getObjectTypeManager()->getObjectTypeByName("Fleet"));
   if(name[0] == '\0' || name[0] == ' ')
      fleet->setName(player->getName() + "'s New Fleet");
   else
      fleet->setName(name);
      
   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());
   fleetData->setSize(2);
   fleetData->setOwner(player->getID());

   // Place the fleet in orbit around the given star
   fleetData->setUnitPos( dynamic_cast<StaticObject*>(starSys->getObjectBehaviour())->getUnitPos());
   fleetData->setVelocity( Vector3d(0LL, 0ll, 0ll));
   
   OrderQueue *fleetoq = new OrderQueue();
   fleetoq->setObjectId(fleet->getID());
   fleetoq->addOwner(player->getID());
   game->getOrderManager()->addOrderQueue(fleetoq);
   OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(
                                 fleet->getParameterByType(obpT_Order_Queue));
   oqop->setQueueId(fleetoq->getQueueId());
   fleetData->setOrderTypes();

   fleet->addToParent(starSys->getID());

   exploreStarSys(fleet);

   return fleet;
}

IGObject* createFleet(Player *player, IGObject* starSys, const string& name,
                      const map<uint32_t, uint32_t>& ships) {
   IGObject* fleet = createEmptyFleet(player, starSys, name);
   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());
   
   for(map<uint32_t, uint32_t>::const_iterator i = ships.begin(); i != ships.end(); ++i)
      fleetData->addShips(i->first, i->second);

   return fleet;
}

pair<IGObject*, bool> createFleet(Player *player, IGObject* starSys, const std::string& name,
                      const map<uint32_t, uint32_t>& ships, Planet *planetData) {
                      
   IGObject* fleet = createEmptyFleet(player, starSys, name);
   Fleet* fleetData = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());

   bool complete = true;

   for(map<uint32_t, uint32_t>::const_iterator i = ships.begin(); i != ships.end(); ++i)
   {
      uint32_t designCost = Rfts::getProductionInfo().getResourceCost(
                  Game::getGame()->getDesignStore()->getDesign(i->first)->getName());

      PlayerInfo &pi = PlayerInfo::getPlayerInfo(player->getID());
      // handle transport creation a lil' different
      if(pi.getTransportId() == i->first)
      {
         // CHECK - this assumes(!) you have enough for a _single_ transport
         // the colonist assumption should be safe due to previous limiting (generate list)
         
         fleetData->addShips(i->first, i->second);
         planetData->removeResource("Resource Point", designCost); // only 1 transport
         planetData->removeResource("Colonist", i->second); // remove colonists
         
      }
      else // normal creation
      {
         uint32_t numShips = i->second;
         unsigned usedRP = designCost * numShips;
         
         if(usedRP > planetData->getCurrentRP())
         {
            complete = false;
            numShips = planetData->getCurrentRP() / designCost;
            usedRP = designCost * numShips;
         }
         
         fleetData->addShips(i->first, numShips);
         planetData->removeResource("Resource Point", usedRP);

         pi.addVictoryPoints(static_cast<uint32_t>(usedRP * 3./4));
      }

   }

   return pair<IGObject*, bool>(fleet, complete);
}


}
