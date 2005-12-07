/*  SplitFleet order
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

#include "order.h"
#include "frame.h"
#include "object.h"
#include "objectmanager.h"
#include "game.h"
#include "logging.h"
#include "fleet.h"
#include "player.h"
#include "message.h"
#include "design.h"
#include "designstore.h"

#include "splitfleet.h"

SplitFleet::SplitFleet() : Order(){
  type = odT_Fleet_Split;
}

SplitFleet::~SplitFleet(){}

void SplitFleet::createFrame(Frame * f, int objID, int pos){
  Order::createFrame(f, objID, pos);
  f->packInt(1); // number of turns
  f->packInt(0); // size of resource list

  Fleet* of = (Fleet*)(Game::getGame()->getObjectManager()->getObject(objID)->getObjectData());

  std::map<int, int> sotf = of->getShips();

  f->packInt(sotf.size());
  DesignStore* ds = Game::getGame()->getDesignStore();

  for(std::map<int, int>::const_iterator itcurr = sotf.begin();
      itcurr != sotf.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packString(ds->getDesign(itcurr->first)->getName().c_str());
    f->packInt(itcurr->second);
  }
 
  f->packInt(ships.size());
  for(std::map<uint32_t, uint32_t>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packInt(itcurr->second);
  }
  Game::getGame()->getObjectManager()->doneWithObject(objID);
}

bool SplitFleet::inputFrame(Frame * f, unsigned int playerid){
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list, should be zero
  // TODO: fix in case size of list is not zero
  f->unpackInt(); // selectable list (shule be zero)
  // TODO: fix in case size of list is not zero

  for(int i = f->unpackInt(); i > 0; i--){
    int type = f->unpackInt();
    int number = f->unpackInt(); // number to shift
    
    ships[type] = number;
  }

  return true;
}

bool SplitFleet::doOrder(IGObject * ob){

  Fleet* of = (Fleet*)(ob->getObjectData());

  Message * msg = new Message();
  msg->setSubject("Split Fleet order complete");
  msg->addReference(rst_Object, ob->getID());

  IGObject * nfleet = Game::getGame() ->getObjectManager()->createNewObject();
  nfleet->setType(4);
  nfleet->setSize(2);
  nfleet->setName("A fleet");
  Fleet* nf = (Fleet*)(nfleet->getObjectData());
  nf->setOwner(of->getOwner());
  nfleet->setPosition(ob->getPosition());
  for(std::map<uint32_t, uint32_t>::iterator scurr = ships.begin(); scurr != ships.end(); ++scurr){
    if(of->removeShips(scurr->first, scurr->second)){
      nf->addShips(scurr->first, scurr->second);
    }else{
      Logger::getLogger()->debug("Player tried to remove too many ships of type %d from fleet", scurr->first);
      //message for player
    }
  }
  
  if(of->totalShips() == 0){
    // whole fleet moved, put it back
    Logger::getLogger()->debug("Whole fleet split, putting it back");
    for(std::map<uint32_t, uint32_t>::iterator scurr = ships.begin(); scurr != ships.end(); ++scurr){
      of->addShips(scurr->first, scurr->second);
    }
    Game::getGame()->getObjectManager()->discardNewObject(nfleet);
    msg->setBody("Fleet not split, not enough ships");
    msg->addReference(rst_Action_Order, rsorav_Incompatible);
  }else if(nf->totalShips() == 0){
    Logger::getLogger()->debug("Split fleet doesn't have any ships, not creating new fleet");
    Game::getGame()->getObjectManager()->discardNewObject(nfleet);
    msg->setBody("Fleet not split, not enough ships");
    msg->addReference(rst_Action_Order, rsorav_Incompatible);
  }else{
    // add fleet to game universe
    Logger::getLogger()->debug("Split fleet successfully");
    msg->setBody("Split fleet complete");
    msg->addReference(rst_Object, nfleet->getID());
    msg->addReference(rst_Action_Order, rsorav_Completion);
    nfleet->addToParent(ob->getParent());
    Game::getGame()->getObjectManager()->addObject(nfleet);
  }
  
  Game::getGame()->getPlayer(nf->getOwner())->postToBoard(msg);
  
  return true;
}

std::map<uint32_t, uint32_t> SplitFleet::getShips() const{
    return ships;
}

void SplitFleet::addShips(uint32_t designid, uint32_t count){
    ships[designid] = count;
}

void SplitFleet::describeOrder(Frame * f) const{
  Order::describeOrder(f);
  f->packString("SplitFleet");
  f->packString("Split the fleet into two");
  f->packInt(1);
  f->packString("ships");
  f->packInt(opT_List);
  f->packString("The ships to be transferred");
  f->packInt64(descmodtime);
}

Order* SplitFleet::clone() const{
  return new SplitFleet();
}
