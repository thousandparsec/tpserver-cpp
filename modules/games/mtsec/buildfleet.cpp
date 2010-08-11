/*  Build object for BuildFleet orders
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

/* TODO GSoC Cleanup Period:
 * Get rid of removeResource, as it's not really needed.  Make it so that
 * resources[1] refers to a name instead of a seemingly arbitrary number,
 * make the code a bit more clear
 */

#include <math.h>
#include <iostream>
#include <boost/bind.hpp>

#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/vector3d.h>
#include "fleet.h"
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

#include "buildfleet.h"

#define MAX(x,y) (x<y) ? (y) : (x)
#define MIN(x,y) (x<y) ? (x) : (y)

namespace MTSecRuleset {

BuildFleet::BuildFleet() : Order()
{
  name = "Build Fleet";
  description = "Build a fleet";
  
  fleetlist = (ListParameter*) addOrderParameter( new ListParameter("ships", "The type of ship to build", boost::bind( &BuildFleet::generateListOptions, this ) ) );
  fleetname = (StringParameter*)addOrderParameter( new StringParameter("name", "The name of the new fleet being built") );
  turns = 1;
}

BuildFleet::~BuildFleet(){
}

void BuildFleet::createFrame(OutputFrame::Ptr f, int pos)
{
  Logger::getLogger()->debug("Enter: BuildFleet::createFrame()");
  // set it to the high end of the production cost... this is a best case scenario where it gets all the factories
  IGObject::Ptr planet = Game::getGame()->getObjectManager()->getObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  
  // number of turns
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > presources = static_cast<Planet*>(planet->getObjectBehaviour())->getResources();
  Game::getGame()->getObjectManager()->doneWithObject(planet->getID());
  uint32_t res_current;
  if(presources.find(1) != presources.end()){
    res_current = presources.find(1)->second.first;
  }else{
    res_current = 0;
  }
  uint32_t usedshipres = resources[1];
  if(pos != 0 || usedshipres == 0){
      turns = usedshipres;
  }else{
    if(usedshipres <= res_current){
      turns = 1;
    }else{
      turns = usedshipres - res_current;
    }
  }
  
  
  Order::createFrame(f, pos);
  Logger::getLogger()->debug("Exit: BuildFleet::createFrame()");

}

ListParameter::Options BuildFleet::generateListOptions(){
  ListParameter::Options options;
  
  IdSet designs = Game::getGame()->getPlayerManager()->getPlayer(((Planet*)(Game::getGame()->getObjectManager()->getObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId())->getObjectBehaviour()))->getOwner())->getPlayerView()->getUsableDesigns();
    Game::getGame()->getObjectManager()->doneWithObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
    DesignStore::Ptr ds = Game::getGame()->getDesignStore();

  std::set<Design::Ptr> usable;
  for(IdSet::iterator itcurr = designs.begin(); itcurr != designs.end(); ++itcurr){
    Design::Ptr design = ds->getDesign(*itcurr);
      if(design->getCategoryId() == 1){
          usable.insert(design);
      }
  }

  for(std::set<Design::Ptr>::iterator itcurr = usable.begin();
      itcurr != usable.end(); ++itcurr){
    Design::Ptr design = (*itcurr);
    options[design->getDesignId()] = ListParameter::Option(design->getName(), 100);
  }

  Logger::getLogger()->debug("Exiting BuildFleet::generateListOptions");
  return options;
}

void BuildFleet::inputFrame(InputFrame::Ptr f, uint32_t playerid)
{
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

  uint32_t total =0;

  IdMap fleettype = fleetlist->getList();
  
  for(IdMap::iterator itcurr = fleettype.begin();
     itcurr != fleettype.end(); ++itcurr){
    uint32_t type = itcurr->first;
    uint32_t number = itcurr->second; // number to build

    if(player->getPlayerView()->isUsableDesign(type) && number >= 0){
      
      Design::Ptr design = ds->getDesign(type);
      total += (int)(ceil(number * design->getPropertyValue(bldTmPropID)));
        design->addUnderConstruction(number);
        ds->designCountsUpdated(design);

    }else{
      throw FrameException( fec_FrameError, "The requested design was not valid.");
    }
  }
  if(total == 0 && !fleettype.empty()){
    throw FrameException( fec_FrameError, "To build was empty...");
  }
  if(resValue != 0){
    turns = (int)(ceil(total/resValue));
  }else{
    turns = 10000;
  }
  resources[1] = total;


  if(fleetname->getString().length() == 0){
      fleetname->setString("A Fleet");
  }
}

bool BuildFleet::doOrder(IGObject::Ptr ob)
{
  Logger::getLogger()->debug("Entering BuildFleet::doOrder");

  Planet* planet = static_cast<Planet*>(ob->getObjectBehaviour());

  Game* game = Game::getGame();
  ResourceManager::Ptr resman = game->getResourceManager();
  const uint32_t resType = resman->getResourceDescription("Factories")->getResourceType();
  const uint32_t resValue = planet->getResourceSurfaceValue(resType);

  int ownerid = planet->getOwner();
  if(ownerid == 0){
      Logger::getLogger()->debug("Exiting BuildFleet::doOrder ownerid == 0");
      //currently not owned by anyone, just forget about it
      return true;
  }

  uint32_t runningTotal = resources[1];
  if (resValue == 0) {
    Message::Ptr msg(new Message());
    msg->setSubject("Build Fleet order error");
    msg->setBody(std::string("The construction of your  new fleet \"") + fleetname->getString() + "\" has been delayed, you do not have any production points this turn.");
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
      msg->setBody(std::string("The construction of your new fleet \"") + fleetname->getString() + "\" has been delayed.");
      Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);
      return false;
    }
  } else if(runningTotal <= resValue && planet->removeResource(resType, runningTotal)){
    //create fleet

    //this is probably unnecessary
    resources[1] = 0;
    Game* game = Game::getGame();
    
    
    IGObject::Ptr fleet = game->getObjectManager()->createNewObject();
    game->getObjectTypeManager()->setupObject(fleet, game->getObjectTypeManager()->getObjectTypeByName("Fleet"));

    //add fleet to container
    fleet->addToParent(ob->getID());

    fleet->setName(fleetname->getString().c_str());

    Fleet * thefleet = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());

    thefleet->setSize(2);
    thefleet->setOwner(ownerid); // set ownerid
    thefleet->setPosition(planet->getPosition());
    thefleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
    
    uint32_t queueid = Game::getGame()->getOrderManager()->addOrderQueue(fleet->getID(), ownerid);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(fleet->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(queueid);
    thefleet->setDefaultOrderTypes();

    //set ship type
    IdMap fleettype = fleetlist->getList();
    for(IdMap::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
      thefleet->addShips(itcurr->first, itcurr->second);
      Design::Ptr design = Game::getGame()->getDesignStore()->getDesign(itcurr->first);
        design->addComplete(itcurr->second);
        Game::getGame()->getDesignStore()->designCountsUpdated(design);
    }
    //add fleet to universe
    Game::getGame()->getObjectManager()->addObject(fleet);
    Game::getGame()->getPlayerManager()->getPlayer(ownerid)->getPlayerView()->addOwnedObject(fleet->getID());

    Message::Ptr msg( new Message() );
    msg->setSubject("Build Fleet order complete");
    msg->setBody(std::string("The construction of your new fleet \"") + fleetname->getString() + "\" is complete.");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, fleet->getID());
    msg->addReference(rst_Object, ob->getID());
 
    Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);
    Logger::getLogger()->debug("Exiting BuildFleet::doOrder on Success");

    return true;
  }
  Logger::getLogger()->debug("Exiting BuildFleet::doOrder on failure");

  return false;
}

bool BuildFleet::removeResource(uint32_t restype, uint32_t amount){
    if(resources.find(restype) != resources.end()){
          if (resources[restype] - amount > 0) {
            resources[restype] -= amount;
            return true;
          }
        }
    return false;
}

Order* BuildFleet::clone() const{
  BuildFleet* nb = new BuildFleet();
  nb->type = type;
  return nb;
}

}
