/*  Rock-Sissor-Paper combat
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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
