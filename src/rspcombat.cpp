
#include "object.h"
#include "fleet.h"
#include "objectdatamanager.h"

#include "rspcombat.h"

RSPCombat::RSPCombat() : CombatStrategy(){
}

RSPCombat::~RSPCombat(){
}

void RSPCombat::doCombat(){
  Fleet * f1, *f2;
  if(c1->getType() == obT_Fleet){
    f1 = (Fleet*)(c1->getObjectData());
  }else if(c1->getType() == obT_Planet){
    f1 = new Fleet();
    f1->addShips(2, 2);
  }
  if(c2->getType() == obT_Fleet){
    f2 = (Fleet*)(c2->getObjectData());
  }else if(c2->getType() == obT_Planet){
    f2 = new Fleet();
    f2->addShips(2, 2);
  }
  if(f1 == NULL || f2 == NULL){
    return;
  }

  

  while(true){
    int r1 = rand() % 3;
    int r2 = rand() % 3;

    int d1, d2;
    
    if(r1 == r2){
      // draw
      d1 = f1->firepower(true);
      d2 = f2->firepower(true);
    }else if(r1 == r2 + 1 || r1 + 2 == r2){
      // f1 won
      d1 = f1->firepower(false);
      if(d1 == 0)
	break;
    }else{
      // f2 won
      d2 = f2->firepower(false);
      if(d2 == 0)
	break;
    }

    bool tte = false;
    if(f1->hit(d2)){
      c1 = NULL;
      tte = true;
    }
    if(f2->hit(d1)){
      c2 = NULL;
      tte = true;
    }
    if(tte)
      break;

  }
  
}
