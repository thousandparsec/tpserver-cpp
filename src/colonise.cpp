

#include "order.h"
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"

#include "colonise.h"

Colonise::Colonise(){
  type = odT_Colonise;
}

Colonise::~Colonise(){}

void Colonise::createFrame(Frame * f, int objID, int pos){
  Order::createFrame(f, objID, pos);
  f->packInt(1); // number of turns
  f->packInt(0); // size of resource list
  f->packInt(planetid);
  
}

bool Colonise::inputFrame(Frame * f){
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list, should be zero
  // TODO: fix in case size of list is not zero
  planetid = f->unpackInt();

  IGObject* target = Game::getGame()->getObject(planetid);

  if(target == NULL || (planetid != 0 && target->getType() != 3)){
    Logger::getLogger()->debug("Player trying to colonise something that is not a planet");
    return false;
  }
  
  return true;
}

bool Colonise::doOrder(IGObject * ob){
  //if not close, move



  return false;
}


void Colonise::describeOrder(Frame * f) const{
  Order::describeOrder(f);
  f->packString("Colonise");
  f->packString("Attempt to colonise a planet");
  f->packInt(1);
  f->packString("planet");
  f->packInt(opT_Object_ID);
  f->packString("The target planet to be colonised");
  
}

Order* Colonise::clone() const{
  return new Colonise();
}
