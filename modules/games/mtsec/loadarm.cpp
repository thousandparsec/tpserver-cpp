/*  Load Armaments order
 *
 *  Copyright (C) 2009 Alan P. Laudicina and the Thousand Parsec Project
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

#include <tpserver/result.h>
#include <tpserver/order.h>
#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/player.h>
#include "fleet.h"
#include "planet.h"
#include <tpserver/message.h>
#include <tpserver/playermanager.h>
#include "mtsecturn.h"
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/listparameter.h>
#include <set>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>

#include "loadarm.h"


namespace MTSecRuleset {

LoadArmament::LoadArmament() : Order()
{
  name = "Load Armament";
  description = "Load a weapon onto your ships";

  weaponlist = new ListParameter();
  weaponlist->setName("Weapons");
  weaponlist->setDescription("The weapon to load");
  weaponlist->setListOptionsCallback(ListOptionCallback(this, &LoadArmament::generateListOptions));
  addOrderParameter(weaponlist);

}

LoadArmament::~LoadArmament(){
}

std::map<uint32_t, std::pair<std::string, uint32_t> > LoadArmament::generateListOptions(){
  Logger::getLogger()->debug("Entering LoadArmament::generateListOptions");
  std::map<uint32_t, std::pair<std::string, uint32_t> > options;

  Game* game = Game::getGame();
  IGObject *selectedObj = game->getObjectManager()->getObject(
            game->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());

  Fleet* fleet = dynamic_cast<Fleet*>(selectedObj->getObjectBehaviour());

  ObjectManager* obman = Game::getGame()->getObjectManager();
  ObjectTypeManager* otman = Game::getGame()->getObjectTypeManager();
  ResourceManager* resman = Game::getGame()->getResourceManager();
  std::set<uint32_t>objs = obman->getObjectsByPos(fleet->getPosition(), 10000);

  for (std::set<uint32_t>::iterator itcurr = objs.begin(); itcurr != objs.end(); ++itcurr) {
    if (obman->getObject(*itcurr)->getType() == otman->getObjectTypeByName("Planet")) {
      IGObject* planetObj = obman->getObject(*itcurr);
      Planet* planet = dynamic_cast<Planet*>(planetObj->getObjectBehaviour());
      Logger::getLogger()->debug("Found Planet %s for Load Armaments Order", planetObj->getName().c_str());
      std::map<uint32_t, std::pair<uint32_t, uint32_t> > reslist = planet->getResources();
      for (std::map<uint32_t, std::pair<uint32_t, uint32_t> >::iterator it = reslist.begin(); it != reslist.end(); ++it) {
        if (resman->getResourceDescription(it->first)->getUnitSingular() == "weapon") {
          options[it->first] = std::pair<std::string, uint32_t>(resman->getResourceDescription(it->first)->getNamePlural(),
            planet->getResourceSurfaceValue(it->first));
        }
      }

    }
  }

  Logger::getLogger()->debug("Exiting LoadArmament::generateListOptions");
  return options;
}


void LoadArmament::createFrame(Frame * f, int pos)
{
  turns = 1;
  Order::createFrame(f, pos);	
}

Result LoadArmament::inputFrame(Frame * f, uint32_t playerid)
{
  return Order::inputFrame(f, playerid);
}

bool LoadArmament::doOrder(IGObject * ob){
  Fleet* fleet = dynamic_cast<Fleet*>(ob->getObjectBehaviour());
  ObjectManager* obman = Game::getGame()->getObjectManager();
  ObjectTypeManager* otman = Game::getGame()->getObjectTypeManager();
  ResourceManager* resman = Game::getGame()->getResourceManager();
  std::set<uint32_t>objs = obman->getObjectsByPos(fleet->getPosition(), 10000);
  IGObject* planetObj;
  Planet* planet;

  for (std::set<uint32_t>::const_iterator itcurr = objs.begin(); itcurr != objs.end(); ++itcurr) {
    if (obman->getObject(*itcurr)->getType() == otman->getObjectTypeByName("Planet")) {

      planetObj = obman->getObject(*itcurr);
      planet = dynamic_cast<Planet*>(planetObj->getObjectBehaviour());
      Logger::getLogger()->debug("LoadArmaments::doOrder Found Planet %s for Load Armaments Order", planetObj->getName().c_str());
      const uint32_t factoryType = resman->getResourceDescription("Factories")->getResourceType();

    std::map<uint32_t,uint32_t> weapontype = weaponlist->getList();
    for(std::map<uint32_t,uint32_t>::iterator weaponit = weapontype.begin(); weaponit != weapontype.end(); ++weaponit) {
      if (planet->removeResource(factoryType, 1)) {
        if (planet->removeResource(weaponit->first, weaponit->second)) {
          Logger::getLogger()->debug("LoadArmaments::doOrder success, adding to resource %d: #:%d", weaponit->first, weaponit->second);
          fleet->addResource(weaponit->first, weaponit->second);
          ob->touchModTime();
          planetObj->touchModTime();
          return true;
          }
        } else {
          turns = 1;
          return true;
        }
      }
    }
  }
  return false;
}

Order* LoadArmament::clone() const{
  LoadArmament *nm = new LoadArmament();
  nm->type = type;
  return nm;
}

}
