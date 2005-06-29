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
#include "game.h"
#include "logging.h"
#include "vector3d.h"
#include "fleet.h"
#include "message.h"
#include "player.h"
#include "design.h"
#include "designstore.h"

#include "ownedobject.h"

#include "build.h"

Build::Build() : Order()
{
  type = odT_Build;
  turnstogo = 0;
}

Build::~Build()
{

}

void Build::createFrame(Frame *f, int objID, int pos)
{
  Order::createFrame(f, objID, pos);

  // number of turns
  f->packInt(turnstogo);     

  f->packInt(0); // size of resource list

  std::set<unsigned int> designs = Game::getGame()->getPlayer(((OwnedObject*)(Game::getGame()->getObject(objID)->getObjectData()))->getOwner())->getUsableDesigns();
  DesignStore* ds = Game::getGame()->getDesignStore(1);

  std::set<unsigned int> dids = ds->getDesignIds();
  std::set<unsigned int> usable;
  set_intersection(designs.begin(), designs.end(), dids.begin(), dids.end(), inserter(usable, usable.begin()));
  
  f->packInt(usable.size());

  for(std::set<unsigned int>::iterator itcurr = usable.begin();
      itcurr != usable.end(); ++itcurr){
    Design * design = ds->getDesign(*itcurr);
    f->packInt(design->getDesignId());
    f->packString(design->getName().c_str());
    f->packInt(100);
  }

  f->packInt(fleettype.size());
  for(std::map<int,int>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packInt(itcurr->second);
  }
}

bool Build::inputFrame(Frame *f, unsigned int playerid)
{
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list (should be zero) TODO
  f->unpackInt(); // selectable list (should be zero) TODO
  
  Player* player = Game::getGame()->getPlayer(playerid);
  DesignStore* ds = Game::getGame()->getDesignStore(1);

  for(int i = f->unpackInt(); i > 0; i--){
    int type = f->unpackInt();
    int number = f->unpackInt(); // number to build
    
    if(player->isUsableDesign(type) && number > 0){
      fleettype[type] = number;

      Design* design = ds->getDesign(type);
      turnstogo += (int)(ceil(number * design->getPropertyValue(1)));

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
    
    IGObject *fleet = new IGObject();

    //add fleet to universe
    Game::getGame()->addObject(fleet);
    //add fleet to container
    ob->addContainedObject(fleet->getID());

    fleet->setSize(2);
    fleet->setType(4);
    fleet->setName("A Fleet");
    ((OwnedObject*)(fleet->getObjectData()))->setOwner(ownerid); // set ownerid
    fleet->setPosition(ob->getPosition());
    fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
   
    
    //set ship type
    Fleet * thefleet = ((Fleet*)(fleet->getObjectData()));
    for(std::map<int,int>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
      thefleet->addShips(itcurr->first, itcurr->second);
    }

    Message * msg = new Message();
    msg->setSubject("Build Fleet order complete");
    msg->setBody("The construction of your new fleet is complete");
    msg->addReference(rst_Action_Order, rsorav_Completion);
    msg->addReference(rst_Object, fleet->getID());
    msg->addReference(rst_Object, ob->getID());
 
    Game::getGame()->getPlayer(ownerid)->postToBoard(msg);

    return true;
  }
  return false;
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
