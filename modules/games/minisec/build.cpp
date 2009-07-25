/*  Build object for BuildFleet orders
 *
 *  Copyright (C) 2004-2005, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/result.h>
#include <tpserver/frame.h>
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

#include "planet.h"
#include "minisec.h"

#include "build.h"

#define MAX(x,y) (x<y) ? (y) : (x)
#define MIN(x,y) (x<y) ? (x) : (y)

Build::Build() : Order()
{
  name = "Build Fleet";
  description = "Build a fleet";
  
  fleetlist = (ListParameter*)  addOrderParameter( new ListParameter("ships","The type of ship to build", boost::bind( &Build::generateListOptions, this )) );
  fleetname = (StringParameter*)addOrderParameter( new StringParameter( "name", "The name of the new fleet being built") );
}

Build::~Build(){
}

void Build::createFrame(Frame *f, int pos)
{
  IGObject * planet = Game::getGame()->getObjectManager()->getObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  
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

}

std::map<uint32_t, std::pair<std::string, uint32_t> > Build::generateListOptions(){
  std::map<uint32_t, std::pair<std::string, uint32_t> > options;
  
  std::set<uint32_t> designs = Game::getGame()->getPlayerManager()->getPlayer(((Planet*)(Game::getGame()->getObjectManager()->getObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId())->getObjectBehaviour()))->getOwner())->getPlayerView()->getUsableDesigns();
    Game::getGame()->getObjectManager()->doneWithObject(Game::getGame()->getOrderManager()->getOrderQueue(orderqueueid)->getObjectId());
  DesignStore* ds = Game::getGame()->getDesignStore();

  std::set<Design*> usable;
  for(std::set<uint32_t>::iterator itcurr = designs.begin(); itcurr != designs.end(); ++itcurr){
      Design* design = ds->getDesign(*itcurr);
      if(design->getCategoryId() == 1){
          usable.insert(design);
      }
  }

  for(std::set<Design*>::iterator itcurr = usable.begin();
      itcurr != usable.end(); ++itcurr){
    Design * design = (*itcurr);
    options[design->getDesignId()] = std::pair<std::string, uint32_t>(design->getName(), 100);
  }
  
  return options;
}

Result Build::inputFrame(Frame *f, uint32_t playerid)
{
  Result r = Order::inputFrame(f, playerid);
  if(!r) return r;
  
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(playerid);
  DesignStore* ds = Game::getGame()->getDesignStore();
  
  uint32_t bldTmPropID = ds->getPropertyByName( "BuildTime");
  
  IdMap fleettype = fleetlist->getList();
  uint32_t usedshipres = 0;
  
  for(IdMap::iterator itcurr = fleettype.begin();
     itcurr != fleettype.end(); ++itcurr){
    uint32_t type = itcurr->first;
    uint32_t number = itcurr->second; // number to build
    
    if(player->getPlayerView()->isUsableDesign(type) && number >= 0){
      
      Design* design = ds->getDesign(type);
      usedshipres += (int)(ceil(number * design->getPropertyValue(bldTmPropID)));
        design->addUnderConstruction(number);
        ds->designCountsUpdated(design);

    }else{
      return Failure("The requested design was not valid.");
    }
  }
  if(usedshipres == 0 && !fleettype.empty()){
    return Failure("To build was empty...");
  }
  
  resources[1] = usedshipres;
  
  if(fleetname->getString().length() == 0){
      fleetname->setString("A Fleet");
  }
  return Success();
}

bool Build::doOrder(IGObject *ob)
{
  
  Planet* planet = static_cast<Planet*>(ob->getObjectBehaviour());

  uint32_t usedshipres = resources[1];
  
  if(usedshipres == 0)
    return true;

  int ownerid = planet->getOwner();
  if(ownerid == 0){
      //currently not owned by anyone, just forget about it
      return true;
  }
  
  planet->addResource(1, 1);
    
  if(planet->removeResource(1, usedshipres)){
    //create fleet
    
    Game* game = Game::getGame();
    
    
    IGObject *fleet = game->getObjectManager()->createNewObject();
    game->getObjectTypeManager()->setupObject(fleet, game->getObjectTypeManager()->getObjectTypeByName("Fleet"));
    
    //add fleet to container
    fleet->addToParent(ob->getID());
    
    fleet->setName(fleetname->getString().c_str());
    
    Fleet * thefleet = ((Fleet*)(fleet->getObjectBehaviour()));
    
    thefleet->setSize(2);
    thefleet->setOwner(ownerid); // set ownerid
    thefleet->setPosition(planet->getPosition());
    thefleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
    
    OrderQueue *fleetoq = new OrderQueue();
    fleetoq->setObjectId(fleet->getID());
    fleetoq->addOwner(ownerid);
    Game::getGame()->getOrderManager()->addOrderQueue(fleetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(fleet->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(fleetoq->getQueueId());
    thefleet->setDefaultOrderTypes();
    thefleet->setIcon("common/object-icons/ship");
    thefleet->setMedia("common-2d/foreign/vegastrike/ship-small/" + ((MiniSec*)(game->getRuleset()))->getFleetMediaNames()->getName());
    
    //set ship type
    IdMap fleettype = fleetlist->getList();
    for(IdMap::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
      thefleet->addShips(itcurr->first, itcurr->second);
        Design* design = Game::getGame()->getDesignStore()->getDesign(itcurr->first);
        design->addComplete(itcurr->second);
        Game::getGame()->getDesignStore()->designCountsUpdated(design);
    }
    //add fleet to universe
    Game::getGame()->getObjectManager()->addObject(fleet);
    Game::getGame()->getPlayerManager()->getPlayer(ownerid)->getPlayerView()->addOwnedObject(fleet->getID());

    Message * msg = new Message();
    msg->setSubject("Build Fleet order complete");
    msg->setBody(std::string("The construction of your new fleet \"") + fleetname->getString() + "\" is complete.");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, fleet->getID());
    msg->addReference(rst_Object, ob->getID());
 
    Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);

    return true;
  }
  return false;
}


Order* Build::clone() const{
  Build* nb = new Build();
  nb->type = type;
  return nb;
}

