
#include "order.h"
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"
#include "fleet.h"
#include "player.h"
#include "message.h"

#include "splitfleet.h"

SplitFleet::SplitFleet(){
  type = odT_Fleet_Split;
}

SplitFleet::~SplitFleet(){}

void SplitFleet::createFrame(Frame * f, int objID, int pos){
  Order::createFrame(f, objID, pos);
  f->packInt(1); // number of turns
  f->packInt(0); // size of resource list

  f->packInt(3);

  Fleet* of = (Fleet*)(Game::getGame()->getObject(objID)->getObjectData());

  f->packInt(0);
  f->packString("Scout");
  f->packInt(of->numShips(0));

  f->packInt(1);
  f->packString("Frigate");
  f->packInt(of->numShips(1));

  f->packInt(2);
  f->packString("Battleship");
  f->packInt(of->numShips(2));

  f->packInt(ships.size());
  for(std::map<int, int>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packInt(itcurr->second);
  }
  
}

bool SplitFleet::inputFrame(Frame * f){
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
  msg->setSubject("Colonised planet");
  msg->setType(0);
  msg->setBody("Split fleet complete");

  IGObject * nfleet = new IGObject();
  Game::getGame()->addObject(nfleet);
  nfleet->setType(4);
  nfleet->setSize(2);
  nfleet->setName("A fleet");
  Fleet* nf = (Fleet*)(nfleet->getObjectData());
  nf->setOwner(of->getOwner());
  nfleet->setPosition(ob->getPosition());
  for(std::map<int, int>::iterator scurr = ships.begin(); scurr != ships.end(); ++scurr){
    if(of->removeShips(scurr->first, scurr->second)){
      nf->addShips(scurr->first, scurr->second);
    }else{
      Logger::getLogger()->debug("Player tried to remove too many ships of type %d from fleet", scurr->first);
      //message for player
    }
  }
  
  if(of->numShips(0) == 0 && of->numShips(1) == 0 && of->numShips(2) == 0){
    // whole fleet moved, put it back
    Logger::getLogger()->debug("Whole fleet split, putting it back");
    for(std::map<int, int>::iterator scurr = ships.begin(); scurr != ships.end(); ++scurr){
      of->addShips(scurr->first, scurr->second);
    }
    Game::getGame()->scheduleRemoveObject(nfleet->getID());
    
  }else if(nf->numShips(0) == 0 && nf->numShips(1) == 0 && nf->numShips(2) == 0){
    Logger::getLogger()->debug("Split fleet doesn't have any ships, not creating new fleet");
    Game::getGame()->scheduleRemoveObject(nfleet->getID());
    
  }else{
    // add fleet to game universe
    Logger::getLogger()->debug("Split fleet successfully");
    Game::getGame()->addObject(nfleet);
    Game::getGame()->getObject(ob->getParent())->addContainedObject(nfleet->getID());
  }
  
  Game::getGame()->getPlayer(nf->getOwner())->postToBoard(msg);
  
  return true;
}

void SplitFleet::describeOrder(Frame * f) const{
  Order::describeOrder(f);
  f->packString("SplitFleet");
  f->packString("Split the fleet into two");
  f->packInt(1);
  f->packString("ships");
  f->packInt(opT_List);
  f->packString("The ships to be transferred");

}

Order* SplitFleet::clone() const{
  return new SplitFleet();
}
