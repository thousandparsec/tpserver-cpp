
#include "frame.h"
#include "order.h"
#include "game.h"
#include "planet.h"
#include "object.h"
#include "objectdatamanager.h"

#include "fleet.h"

Fleet::Fleet():OwnedObject()
{
  damage = 0;
}

Fleet::~Fleet(){
}

void Fleet::addShips(int type, int number){
  ships[type] += number;
}

bool Fleet::removeShips(int type, int number){
  if(ships[type] >= number){
    ships[type] -= number;
    return true;
  }
  return false;
}

int Fleet::numShips(int type){
  return ships[type];
}

long long Fleet::maxSpeed(){
  if(ships[2] > 0){
    return 100000000;
  }else if(ships[1] > 0){
    return 200000000;
  }else{
    return 300000000;
  }
}

int Fleet::firepower(bool draw){
  int fp = 0;
  if(draw){
    fp += ships[2];
  }else{
    fp += ships[2] * 3;
    fp += ships[1] * 2;
  }
  return fp;
}

bool Fleet::hit(int firepower){
  damage += firepower;
  bool change = true;
  while(change){
    change = false;
    if(ships[2] > 0 && damage >= 6){
      ships[2]--;
      damage -= 6;
      change = true;
    }else if(ships[1] > 0 && damage >= 4){
      ships[1]--;
      damage -= 4;
      change = true;
    }else if(ships[0] > 0 && damage > 2){
      ships[0]--;
      damage -= 2;
      change = true;
    }
  }
  return (ships[2] == 0 && ships[1] == 0 && ships[0] == 0);
}

void Fleet::packExtraData(Frame * frame)
{
	OwnedObject::packExtraData(frame);
	
	frame->packInt(ships.size());
	for(std::map<int, int>::iterator itcurr = ships.begin(); itcurr != ships.end(); ++itcurr){
	  //if(itcurr->second > 0){
	    frame->packInt(itcurr->first);
	    frame->packInt(itcurr->second);
	    //}
	}
	
	frame->packInt(damage);

}

void Fleet::doOnceATurn(IGObject * obj)
{
  IGObject * pob = Game::getGame()->getObject(obj->getParent());
  if(pob->getType() == obT_Planet && ((Planet*)(pob->getObjectData()))->getOwner() == getOwner()){
    damage = 0;
  }
}

void Fleet::packAllowedOrders(Frame * frame, int playerid){
  if(playerid == getOwner()){
    if(ships.find(1) != ships.end()){
      frame->packInt(5);
      frame->packInt(odT_Colonise);
    }else{
      frame->packInt(4);
    }
    frame->packInt(odT_Move);
    frame->packInt(odT_Nop);
    frame->packInt(odT_Fleet_Split);
    frame->packInt(odT_Fleet_Merge);
    
  }else{
    frame->packInt(0);
  }
}

bool Fleet::checkAllowedOrder(int ot, int playerid){
  return (playerid == getOwner() && (ot == odT_Move || ot == odT_Nop || ot == odT_Fleet_Split || ot == odT_Fleet_Merge || (ships.find(1) != ships.end() && ot == odT_Colonise)));
}

int Fleet::getContainerType(){
  return 0;
}

ObjectData* Fleet::clone(){
  return new Fleet();
}
