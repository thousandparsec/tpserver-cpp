

#include "order.h"
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"
#include "fleet.h"
#include "move.h"

#include "mergefleet.h"

MergeFleet::MergeFleet(){
  type = odT_Fleet_Merge;
  moveorder = new Move();
}

MergeFleet::~MergeFleet(){
  delete moveorder;
}

void MergeFleet::createFrame(Frame * f, int objID, int pos){
  Order::createFrame(f, objID, pos);
  moveorder->setDest(Game::getGame()->getObject(fleetid)->getPosition());
  f->packInt(moveorder->getETA(Game::getGame()->getObject(objID))); // number of turns
  f->packInt(0); // size of resource list
  f->packInt(fleetid);
}

bool MergeFleet::inputFrame(Frame * f){
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list, should be zero
  // TODO: fix in case size of list is not zero
  fleetid = f->unpackInt();

  IGObject* target = Game::getGame()->getObject(fleetid);

  if(target == NULL || (fleetid != 0 && target->getType() != 4) /*|| ((Fleet*)(target->getObjectData()))->getOwner() != this object's owner */){
    Logger::getLogger()->debug("Player tried to merge fleet with something that is not a fleet");
    return false;
  }
  moveorder->setDest(target->getPosition());

  return true;
}


bool MergeFleet::doOrder(IGObject * ob){
  moveorder->setDest(Game::getGame()->getObject(fleetid)->getPosition());
  
  if(moveorder->doOrder(ob)){

    Fleet *myfleet = (Fleet*)(ob->getObjectData());
    if(fleetid != 0){
      Fleet *tfleet = (Fleet*)(Game::getGame()->getObject(fleetid)->getObjectData());
      
      if(tfleet->getOwner() == myfleet->getOwner()){
	tfleet->addShips(0, myfleet->numShips(0));
	tfleet->addShips(1, myfleet->numShips(1));
	tfleet->addShips(2, myfleet->numShips(2));
	
	Game::getGame()->scheduleRemoveObject(ob->getID());
	
      }
    }
    
    return true;
  }else{
    return false;
  }
}

void MergeFleet::describeOrder(Frame * f) const{
  Order::describeOrder(f);
  f->packString("MergeFleet");
  f->packString("Merge this fleet into another one");
  f->packInt(1);
  f->packString("fleet");
  f->packInt(opT_Object_ID);
  f->packString("The fleet to be merged into");

}

Order* MergeFleet::clone() const{
  return new MergeFleet();
}
