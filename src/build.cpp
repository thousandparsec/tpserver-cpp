
#include "frame.h"
#include "object.h"
#include "game.h"
#include "logging.h"
#include "vector3d.h"
#include "fleet.h"

#include "ownedobject.h"

#include "build.h"

Build::Build()
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

  f->packInt(3);

  f->packInt(0);
  f->packString("Scout");
  f->packInt(100);

  f->packInt(1);
  f->packString("Frigate");
  f->packInt(100);

  f->packInt(2);
  f->packString("Battleship");
  f->packInt(100);

  f->packInt(fleettype.size());
  for(std::map<int,int>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
    f->packInt(itcurr->first);
    f->packInt(itcurr->second);
  }
}

bool Build::inputFrame(Frame *f)
{
  f->unpackInt(); // number of turns
  f->unpackInt(); // size of resource list (should be zero) TODO
  f->unpackInt(); // selectable list (should be zero) TODO
  
  for(int i = f->unpackInt(); i > 0; i--){
    int type = f->unpackInt();
    int number = f->unpackInt(); // number to build
    
    fleettype[type] = number;

    switch(type){
    case 0:
      turnstogo += 1 * number;
      break;
    case 1:
      turnstogo += 2 * number;
      break;
    case 2:
      turnstogo += 4 * number;
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
    fleet->setPosition(ob->getPosition());
    fleet->setVelocity(Vector3d(0LL, 0ll, 0ll));
   
    
    //set ship type
    Fleet * thefleet = ((Fleet*)(fleet->getObjectData()));
    for(std::map<int,int>::iterator itcurr = fleettype.begin(); itcurr != fleettype.end(); ++itcurr){
      thefleet->addShips(itcurr->first, itcurr->second);
    }

    return true;
  }
  return false;
}

void Build::describeOrder(int orderType, Frame *f)
{
  if(orderType == odT_Build){
    f->packInt(odT_Build);
    f->packString("BuildFleet");
    f->packString("Build something");
    f->packInt(1); // num params
    f->packString("ships");
    f->packInt(6);
    f->packString("The type of ship to build");
  }
}
