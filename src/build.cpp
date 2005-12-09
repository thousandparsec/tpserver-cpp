/*  Build object for BuildFleet orders
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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

#include "frame.h"
#include "object.h"
#include "objectmanager.h"
#include "game.h"
#include "logging.h"
#include "vector3d.h"
#include "fleet.h"
#include "message.h"
#include "player.h"
#include "design.h"
#include "designstore.h"
#include "playermanager.h"

#include "ownedobject.h"

#include "build.h"

Build::Build() : Order()
{
  type = odT_Build;
  turnstogo = 0;
}

Build::~Build()
{
    for(std::map<uint32_t,uint32_t>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
        Design* design = Game::getGame()->getDesignStore()->getDesign(itcurr->first);
        design->removeCanceledConstruction(itcurr->second);
        Game::getGame()->getDesignStore()->designCountsUpdated(design);
    }
}

void Build::createFrame(Frame *f, int objID, int pos)
{
  Order::createFrame(f, objID, pos);

  // number of turns
  f->packInt(turnstogo);     

  f->packInt(0); // size of resource list

  std::set<unsigned int> designs = Game::getGame()->getPlayerManager()->getPlayer(((OwnedObject*)(Game::getGame()->getObjectManager()->getObject(objID)->getObjectData()))->getOwner())->getUsableDesigns();
    Game::getGame()->getObjectManager()->doneWithObject(objID);
  DesignStore* ds = Game::getGame()->getDesignStore();

  std::set<Design*> usable;
    for(std::set<uint>::iterator itcurr = designs.begin(); itcurr != designs.end(); ++itcurr){
        Design* design = ds->getDesign(*itcurr);
        if(design->getCategoryId() == 1){
            usable.insert(design);
        }
    }
  
  f->packInt(usable.size());
    Logger::getLogger()->debug("There are %d designs in the usable list", usable.size());

  for(std::set<Design*>::iterator itcurr = usable.begin();
      itcurr != usable.end(); ++itcurr){
    Design * design = (*itcurr);
    f->packInt(design->getDesignId());
    f->packString(design->getName().c_str());
    f->packInt(100);
  }

  f->packInt(fleettype.size());
  for(std::map<uint32_t,uint32_t>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packInt(itcurr->second);
  }
}

bool Build::inputFrame(Frame *f, unsigned int playerid)
{
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list (should be zero) TODO
  f->unpackInt(); // selectable list (should be zero) TODO
  
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(playerid);
  DesignStore* ds = Game::getGame()->getDesignStore();

  for(int i = f->unpackInt(); i > 0; i--){
    uint32_t type = f->unpackInt();
    uint32_t number = f->unpackInt(); // number to build
    
    if(player->isUsableDesign(type) && number > 0){
      fleettype[type] = number;

      Design* design = ds->getDesign(type);
      turnstogo += (int)(ceil(number * design->getPropertyValue(2)));
        design->addUnderConstruction(number);
        ds->designCountsUpdated(design);

    }
  }
  return true;
}

bool Build::doOrder(IGObject *ob)
{
  turnstogo--;

  if(turnstogo <= 0){
    int ownerid = ((OwnedObject*)(ob->getObjectData()))->getOwner();
		   
    //create fleet
    
    IGObject *fleet = Game::getGame()->getObjectManager()->createNewObject();

    
    //add fleet to container
    fleet->addToParent(ob->getID());

    fleet->setSize(2);
    fleet->setType(4);
    fleet->setName("A Fleet");
    ((OwnedObject*)(fleet->getObjectData()))->setOwner(ownerid); // set ownerid
    fleet->setPosition(ob->getPosition());
    fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
    
    //set ship type
    Fleet * thefleet = ((Fleet*)(fleet->getObjectData()));
    for(std::map<uint32_t,uint32_t>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
      thefleet->addShips(itcurr->first, itcurr->second);
        Design* design = Game::getGame()->getDesignStore()->getDesign(itcurr->first);
        design->addComplete(itcurr->second);
        Game::getGame()->getDesignStore()->designCountsUpdated(design);
    }
    //add fleet to universe
    Game::getGame()->getObjectManager()->addObject(fleet);

    Message * msg = new Message();
    msg->setSubject("Build Fleet order complete");
    msg->setBody("The construction of your new fleet is complete");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, fleet->getID());
    msg->addReference(rst_Object, ob->getID());
 
    Game::getGame()->getPlayerManager()->getPlayer(ownerid)->postToBoard(msg);

    return true;
  }
  return false;
}

 uint32_t Build::getTimeToGo() const{
    return turnstogo;
}

std::map<uint32_t, uint32_t> Build::getShips() const{
    return fleettype;
}

void Build::setTimeToGo(uint32_t ttg){
    turnstogo = ttg;
}

void Build::addShips(uint32_t designid, uint32_t count){
    fleettype[designid] = count;
}

void Build::describeOrder(Frame *f) const
{
  Order::describeOrder(f);
  f->packString("BuildFleet");
  f->packString("Build something");
  f->packInt(1); // num params
  f->packString("ships");
  f->packInt(6);
  f->packString("The type of ship to build");
  f->packInt64(descmodtime);

}

Order* Build::clone() const{
  return new Build();
  // should probably copy the type field of Order
}
