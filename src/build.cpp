
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"

#include "ownedobject.h"

#include "build.h"

Build::Build()
{
  type = odT_Build;
  fleettype = 0;
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

  f->packInt(3);

  f->packInt(0);
  f->packString("Scout");
  f->packInt(1);

  f->packInt(1);
  f->packString("Frigate");
  f->packInt(1);

  f->packInt(2);
  f->packString("Battleship");
  f->packInt(1);

  f->packInt(1);
  f->packInt(fleettype);
  f->packInt(1);
}

bool Build::inputFrame(Frame *f)
{
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list (should be zero) TODO
  f->unpackInt(); // selectable list (should be zero) TODO
  
  for(int i = f->unpackInt(); i > 0; i--){

    fleettype = f->unpackInt();
    f->unpackInt(); // number to build
    
    switch(fleettype){
    case 0:
      turnstogo += 1;
      break;
    case 1:
      turnstogo += 2;
      break;
    case 2:
      turnstogo += 4;
      break;
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
    fleet->setPosition3(ob->getPositionX(), ob->getPositionY(),ob->getPositionZ());
    fleet->setVelocity3(0LL, 0ll, 0ll);
    fleet->addAction(-1, ownerid, odT_Move); // let ship move
    fleet->addAction(-1, ownerid, odT_Nop); // let ship stop
    
    //set ship type

    return true;
  }
  return false;
}

void Build::describeOrder(int orderType, Frame *f)
{
  if(orderType == odT_Build){
    f->packInt(odT_Build);
    f->packString("Build");
    f->packString("Build something");
    f->packInt(1); // num params
    f->packString("type");
    f->packInt(7);
    f->packString("The type of ship to build");
  }
}
