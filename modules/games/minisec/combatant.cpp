/*  Combatant object
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <math.h>

#include <tpserver/game.h>
#include "planet.h"
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>

#include "combatant.h"

Combatant::Combatant() : playerref(0), shiplist(), damage(0){
}

Combatant::~Combatant(){
}

uint32_t Combatant::getOwner() const{
  return playerref;
}

void Combatant::setOwner(uint32_t no){
  playerref = no;
}

void Combatant::addShips(uint32_t type, uint32_t number){
  shiplist[type] += number;
}

bool Combatant::removeShips(uint32_t type, uint32_t number){
  if(shiplist[type] >= number){
    shiplist[type] -= number;
    return true;
  }
  return false;
}

uint32_t Combatant::numShips(uint32_t type){
  return shiplist[type];
}

std::map<uint32_t, uint32_t> Combatant::getShips() const{
  return shiplist;
}

uint32_t Combatant::totalShips() const{
  uint32_t num = 0;
  for(std::map<uint32_t, uint32_t>::const_iterator itcurr = shiplist.begin();
      itcurr != shiplist.end(); ++itcurr){
    num += itcurr->second;
  }
  return num;
}

std::list<uint32_t> Combatant::firepower(bool draw){
  std::list<uint32_t> fp;
  DesignStore* ds = Game::getGame()->getDesignStore();
  for(std::map<uint32_t, uint32_t>::iterator itcurr = shiplist.begin();
      itcurr != shiplist.end(); ++itcurr){
    int attnum;
    if(draw){
      attnum = (uint32_t)(ds->getDesign(itcurr->first)->getPropertyValue(ds->getPropertyByName("WeaponDraw")));
    }else{
      attnum = (uint32_t)(ds->getDesign(itcurr->first)->getPropertyValue(ds->getPropertyByName("WeaponWin")));
    }
    for(uint32_t i = 0; i < itcurr->second; i++){
      fp.push_back(attnum);
    }
  }
  return fp;
}

bool Combatant::hit(std::list<uint32_t> firepower){
  DesignStore* ds = Game::getGame()->getDesignStore();
  for(std::list<uint32_t>::iterator shot = firepower.begin(); shot != firepower.end(); ++shot){
    uint32_t shiptype = 0;
    uint32_t shiphp = 0;
    
    uint32_t armourprop = ds->getPropertyByName("Armour");
    if(armourprop == 0){
      armourprop = ds->getPropertyByName("Amour");
    }
    
    for(std::map<uint32_t, uint32_t>::iterator itcurr = shiplist.begin();
        itcurr != shiplist.end(); ++itcurr){
      if(itcurr->second == 0)
        continue;
      Design *design = ds->getDesign(itcurr->first);
      if(shiphp < (uint32_t)design->getPropertyValue(armourprop)){
        shiptype = itcurr->first;
        shiphp = (uint32_t)design->getPropertyValue(armourprop);
      }
    }
    if(shiphp == 0){
      return false;
    }
    //get the current damage
    uint32_t ldam = damage / shiplist[shiptype];
    if(ldam + (*shot) >= shiphp){
      shiplist[shiptype]--;
      damage -= ldam;
      Design* design = ds->getDesign(shiptype);
      design->removeDestroyed(1);
      ds->designCountsUpdated(design);
    }else{
      damage += (*shot);
    }
  }
  if(totalShips() == 0)
    return false;
  else
    return true;
}

uint32_t Combatant::getDamage() const{
    return damage;
}

void Combatant::setDamage(uint32_t nd){
    damage = nd;
}
