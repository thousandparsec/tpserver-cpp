/*  Build object for BuildWeapon orders
 *
 *  Copyright (C) 2009  Alan P. Laudicina and the Thousand Parsec Project
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
#include <iostream>

#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/vector3d.h>
#include <tpserver/message.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/playermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/ordermanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/objectparametergroupdesc.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/integerobjectparam.h>
#include <tpserver/orderparameters.h>

#include "planet.h"

#include "buildweapon.h"

#define MAX(x,y) (x<y) ? (y) : (x)
#define MIN(x,y) (x<y) ? (x) : (y)

namespace MTSecRuleset {

BuildWeapon::BuildWeapon() : Order()
{
  name = "Build Weapon";
  description = "Build a Weapon";

  weaponlist = (ListParameter*) addOrderParameter( new ListParameter("Weapons", "The type of weapon to build", boost::bind( &BuildWeapon::generateListOptions, this ) ) );

  turns = 1;
}

BuildWeapon::~BuildWeapon(){
}

void BuildWeapon::createFrame(OutputFrame::Ptr f, int pos)
{
  Logger::getLogger()->debug("Enter: BuildWeapon::createFrame()");
  Order::createFrame(f, pos);
  Logger::getLogger()->debug("Exit: BuildWeapon::createFrame()");

}

std::map<uint32_t, std::pair<std::string, uint32_t> > BuildWeapon::generateListOptions(){
  Logger::getLogger()->debug("Entering BuildWeapon::generateListOptions");
  std::map<uint32_t, std::pair<std::string, uint32_t> > options;
  
  std::set<uint32_t> designs = Game::getGame()->getPlayerManager()->getPlayer((dynamic_cast<Planet*>(Game::getGame()->getObjectManager()->getObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId())->getObjectBehaviour()))->getOwner())->getPlayerView()->getUsableDesigns();

    Game::getGame()->getObjectManager()->doneWithObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  DesignStore::Ptr ds = Game::getGame()->getDesignStore();

  std::set<Design::Ptr> usable;
  for(std::set<uint32_t>::iterator itcurr = designs.begin(); itcurr != designs.end(); ++itcurr){
      Design::Ptr design = ds->getDesign(*itcurr);
      if(design->getCategoryId() == 2){
          usable.insert(design);
      }
  }

  for(std::set<Design::Ptr>::iterator itcurr = usable.begin();
      itcurr != usable.end(); ++itcurr){
    Design::Ptr design = (*itcurr);
    options[design->getDesignId()] = std::pair<std::string, uint32_t>(design->getName(), 100);
  }

  Logger::getLogger()->debug("Exiting BuildWeapon::generateListOptions");
  return options;
}

void BuildWeapon::inputFrame(InputFrame::Ptr f, uint32_t playerid)
{
  Logger::getLogger()->debug("Enter: BuildWeapon::inputFrame()");
  Order::inputFrame(f, playerid);

  Game* game = Game::getGame();

  Player::Ptr player = game->getPlayerManager()->getPlayer(playerid);
  DesignStore::Ptr ds = game->getDesignStore();
  ResourceManager::Ptr resman = game->getResourceManager();
  IGObject::Ptr igobj = game->getObjectManager()->getObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  Planet * planet = dynamic_cast<Planet*>(igobj->getObjectBehaviour());

  const uint32_t resType = resman->getResourceDescription("Factories")->getResourceType();
  const uint32_t resValue = planet->getResourceSurfaceValue(resType);
  uint32_t bldTmPropID = ds->getPropertyByName("ProductCost");

  uint32_t total = 0;

  IdMap weapontype = weaponlist->getList();

  for(IdMap::iterator itcurr = weapontype.begin(); itcurr != weapontype.end(); ++itcurr){
    uint32_t type = itcurr->first;
    uint32_t number = itcurr->second; // number to build

    if(player->getPlayerView()->isUsableDesign(type) && number >= 0){
      Design::Ptr design = ds->getDesign(type);
      design->addUnderConstruction(number);
      ds->designCountsUpdated(design);
      total += (int)(ceil(number * design->getPropertyValue(bldTmPropID)));
    }else{
        throw FrameException( fec_FrameError, "The requested design was not valid.");
    }
  }
  turns = (int)(ceil(total/resValue));
  resources[1] = total;

}

bool BuildWeapon::doOrder(IGObject::Ptr ob)
{
  Logger::getLogger()->debug("Entering BuildWeapon::doOrder");

  Planet* planet = static_cast<Planet*>(ob->getObjectBehaviour());

  Game* game = Game::getGame();
  ResourceManager::Ptr resman = game->getResourceManager();
  const uint32_t resType = resman->getResourceDescription("Factories")->getResourceType();
  const uint32_t resValue = planet->getResourceSurfaceValue(resType);

  int ownerid = planet->getOwner();
  if(ownerid == 0){
      Logger::getLogger()->debug("Exiting BuildWeapon::doOrder ownerid == 0");
      //currently not owned by anyone, just forget about it
      return true;
  }

  uint32_t runningTotal = resources[1];

  if (resValue == 0) {
    Message::Ptr msg(new Message());
    msg->setSubject("Build Fleet order error");
    msg->setBody(std::string("The construction of your new weapon has been delayed, you do not have any production points this turn."));
    Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);
    return false;
  } else if(runningTotal > resValue) {
    if (planet->removeResource(resType, resValue)) {
      removeResource(1, resValue);
      runningTotal = resources[1];
      uint32_t planetFactories = planet->getFactoriesPerTurn();
      turns = static_cast<uint32_t>(ceil(runningTotal / planetFactories));
      Message::Ptr msg(new Message());
      msg->setSubject("Build Fleet order slowed");
      msg->setBody(std::string("The construction of your new weapon has been delayed."));
      Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);
      return false;
    }
  } else if(runningTotal <= resValue && planet->removeResource(resType, runningTotal)){
    //create weapon

    //this is probably unnecessary
    resources[1] = 0;
    Game* game = Game::getGame();

    //create the resource on the planet
    ResourceManager::Ptr resman = game->getResourceManager();

    IdMap weapontype = weaponlist->getList();
    for(IdMap::iterator itcurr = weapontype.begin(); itcurr != weapontype.end(); ++itcurr) {
      Design::Ptr design = game->getDesignStore()->getDesign(itcurr->first);
      if (resman->getResourceDescription(design->getName()) == NULL) {
        ResourceDescription::Ptr res(new ResourceDescription());
        res->setNameSingular(design->getName().c_str());
        res->setNamePlural(design->getName().c_str());
        res->setUnitSingular("weapon");
        res->setUnitPlural("weapons");
        res->setDescription("A weapon");
        res->setMass(0);
        res->setVolume(0);
        resman->addResourceDescription(res);
        Logger::getLogger()->debug("Added Resource %s", design->getName().c_str());
      }
      uint32_t resNum = resman->getResourceDescription(design->getName())->getResourceType();
      Logger::getLogger()->debug("Resource Type(%d)", resNum);
      planet->addResource(resNum, itcurr->second);
    }

    Message::Ptr msg(new Message());
    msg->setSubject("Build Weapon order complete");
    msg->setBody(std::string("The construction of your new weapon is complete."));
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, ob->getID());
 
    game->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);
    Logger::getLogger()->debug("Exiting BuildWeapon::doOrder on Success");
    ob->touchModTime();
    return true;
  }
  Logger::getLogger()->debug("Exiting BuildWeapon::doOrder on failure");

  return false;
}

bool BuildWeapon::removeResource(uint32_t restype, uint32_t amount){
    if(resources.find(restype) != resources.end()){
          if (resources[restype] - amount > 0) {
            resources[restype] -= amount;
            return true;
          }
        }
    return false;
}

Order* BuildWeapon::clone() const{
  BuildWeapon* nb = new BuildWeapon();
  nb->type = type;
  return nb;
}

}
