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

#include <tpserver/frame.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/vector3d.h>
#include "fleet.h"
#include <tpserver/message.h>
#include <tpserver/player.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/playermanager.h>

#include "planet.h"

#include "build.h"

#define MAX(x,y) (x<y) ? (y) : (x)
#define MIN(x,y) (x<y) ? (x) : (y)

Build::Build() : Order(), fleettype(), fname()
{
  type = odT_Build;
  turnstogo = 0;
    usedshipres = 0;
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

  IGObject * planet = Game::getGame()->getObjectManager()->getObject(objID);
    if(turnstogo != 0)
        update(planet);

    // number of turns
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > resources = static_cast<Planet*>(planet->getObjectData())->getResources();
    Game::getGame()->getObjectManager()->doneWithObject(objID);
    uint32_t res_current;
    if(resources.find(1) != resources.end()){
      res_current = resources.find(1)->second.first;
    }else{
      res_current = 0;
    }
    if(pos != 0 || usedshipres == 0){
        f->packInt(usedshipres);
    }else{
      if(usedshipres <= res_current){
        f->packInt(1);
      }else{
        f->packInt(usedshipres - res_current);
      }
    }

  f->packInt(1); // size of resource list
  f->packInt(1); // ship part resources
  f->packInt(usedshipres);

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
    f->packInt(1024);
    f->packString(fname.c_str());
}

bool Build::inputFrame(Frame *f, unsigned int playerid)
{
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list (should be zero) TODO
  f->unpackInt(); // selectable list (should be zero) TODO
  
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(playerid);
  DesignStore* ds = Game::getGame()->getDesignStore();
  
  unsigned int bldTmPropID = ds->getPropertyByName( "BuildTime");
  
  for(int i = f->unpackInt(); i > 0; i--){
    uint32_t type = f->unpackInt();
    uint32_t number = f->unpackInt(); // number to build
    
    if(player->isUsableDesign(type) && number > 0){
      fleettype[type] = number;

      Design* design = ds->getDesign(type);
      usedshipres += (int)(ceil(number * design->getPropertyValue(bldTmPropID)));
        design->addUnderConstruction(number);
        ds->designCountsUpdated(design);

    }
  }
    f->unpackInt();
    fname = f->unpackString();
    if(fname.length() == 0){
        fname = "A Fleet";
    }
  return true;
}

bool Build::doOrder(IGObject *ob)
{
    
    if(turnstogo != 0)
        update(ob);
    
    Planet* planet = static_cast<Planet*>(ob->getObjectData());
  
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
    
    IGObject *fleet = Game::getGame()->getObjectManager()->createNewObject();

    
    //add fleet to container
    fleet->addToParent(ob->getID());

    fleet->setSize(2);
    fleet->setType(4);
    fleet->setName(fname.c_str());
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
    msg->setBody(std::string("The construction of your new fleet \"") + fname + "\" is complete");
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

uint32_t Build::getRequiredResources() const{
    return usedshipres;
}

std::string Build::getFleetName() const{
    return fname;
}

void Build::setTimeToGo(uint32_t ttg){
    turnstogo = ttg;
}

void Build::addShips(uint32_t designid, uint32_t count){
    fleettype[designid] = count;
}

void Build::setRequiredResources(uint32_t spart_res){
    usedshipres = spart_res;
}

void Build::setFleetName(const std::string& name){
    fname = name;
}

void Build::describeOrder(Frame *f) const
{
  Order::describeOrder(f);
  f->packString("BuildFleet");
  f->packString("Build something");
    f->packInt(2); // num params
  f->packString("ships");
  f->packInt(6);
  f->packString("The type of ship to build");
    f->packString("name");
    f->packInt(opT_String);
    f->packString("The name of the new fleet being built");
  f->packInt64(descmodtime);

}

Order* Build::clone() const{
  return new Build();
  // should probably copy the type field of Order
}

void Build::update(IGObject* planet){
    DesignStore *ds = Game::getGame()->getDesignStore();
    for(std::map<uint32_t, uint32_t>::iterator itcurr = fleettype.begin();
            itcurr != fleettype.end(); ++itcurr){
        Design* design = ds->getDesign(itcurr->first);
        usedshipres += (int)(ceil(itcurr->second * design->getPropertyValue(2)));
    }
    static_cast<Planet*>(planet->getObjectData())->addResource(1, MIN(0, usedshipres - MAX(0, turnstogo)));
    turnstogo = 0;
}
