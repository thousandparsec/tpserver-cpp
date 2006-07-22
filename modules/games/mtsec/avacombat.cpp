/*
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

#include <tpserver/object.h>
#include "fleet.h"
#include <tpserver/objectdatamanager.h>
#include <tpserver/message.h>
#include <tpserver/game.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>

#include "avacombat.h"

AVACombat::AVACombat() : CombatStrategy(){
}

AVACombat::~AVACombat(){
}

void AVACombat::doCombat(){
  Fleet * f1, *f2;
  if(c1->getType() == obT_Fleet){
    f1 = (Fleet*)(c1->getObjectData());
  }else if(c1->getType() == obT_Planet){
    f1 = new Fleet();
    f1->addShips(3, 2);
    f1->setOwner(((OwnedObject*)c1->getObjectData())->getOwner());
  }
  if(c2->getType() == obT_Fleet){
    f2 = (Fleet*)(c2->getObjectData());
  }else if(c2->getType() == obT_Planet){
    f2 = new Fleet();
    f2->addShips(3, 2);
    f2->setOwner(((OwnedObject*)c2->getObjectData())->getOwner());
  }
  if(f1 == NULL || f2 == NULL){
    return;
  }

  Message *msg1, *msg2;
  msg1 = new Message();
  msg2 = new Message();
  msg1->setSubject("Combat");
  msg2->setSubject("Combat");
  msg1->addReference(rst_Object, c1->getID());
  msg1->addReference(rst_Object, c2->getID());
  msg1->addReference(rst_Player, f2->getOwner());
  msg2->addReference(rst_Object, c2->getID());
  msg2->addReference(rst_Object, c1->getID());
  msg2->addReference(rst_Player, f1->getOwner());

  while(true){
    int r1 = rand() % 3;
    int r2 = rand() % 3;

    int d1 = 0, d2 = 0;

    if(r1 == r2){
      // draw
      d1 = f1->firepower(true);
      d2 = f2->firepower(true);
    }else if(r1 == r2 + 1 || r1 + 2 == r2){
      // f1 won
      d1 = f1->firepower(false);
      if(d1 == 0){
	msg1->setBody("Your fleet escaped");
	msg2->setBody("Their fleet escaped");
	break;
      }
    }else{
      // f2 won
      d2 = f2->firepower(false);
      if(d2 == 0){
	msg1->setBody("Their fleet escaped");
	msg2->setBody("Your fleet escaped");
	break;
      }
    }

    bool tte = false;

    std::string body1, body2;

    if(!f1->hit(d2)){
      body1 += "Your fleet was destroyed. ";
      body2 += "You destroyed their fleet. ";
      c1 = NULL;
      tte = true;
    }
    if(!f2->hit(d1)){
      body2 += "Your fleet was destroyed.";
      body1 += "You destroyed their fleet.";
      c2 = NULL;
      tte = true;
    }
    if(tte){
      msg1->setBody(body1);
      msg2->setBody(body2);
      break;
    }

  }
  Game::getGame()->getPlayerManager()->getPlayer(f1->getOwner())->postToBoard(msg1);
  Game::getGame()->getPlayerManager()->getPlayer(f2->getOwner())->postToBoard(msg2);
}
