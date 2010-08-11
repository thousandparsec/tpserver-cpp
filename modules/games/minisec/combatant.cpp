/*  Combatant object
 *
 *  Copyright (C) 2004-2005, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/game.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>

#include "combatant.h"

Combatant::Combatant() : BattleXML::Combatant(), playerref(0), objectref(0), shiptype(0), damage(0){
}

Combatant::~Combatant(){
}

playerid_t Combatant::getOwner() const{
  return playerref;
}

void Combatant::setOwner(playerid_t no){
  playerref = no;
}

objectid_t Combatant::getObject() const{
    return objectref;
}

void Combatant::setObject(objectid_t no){
    objectref = no;
}

void Combatant::setShipType(designid_t type){
  shiptype = type;
}

designid_t Combatant::getShipType() const{
  return shiptype;
}

uint32_t Combatant::firepower(bool draw){
    if(shiptype == 0){
        return draw ? 1 : 3;
    }
    
    DesignStore::Ptr ds = Game::getGame()->getDesignStore();
    
    uint32_t attnum;
    if(draw){
        attnum = (uint32_t)(ds->getDesign(shiptype)->getPropertyValue(ds->getPropertyByName("WeaponDraw")));
    }else{
        attnum = (uint32_t)(ds->getDesign(shiptype)->getPropertyValue(ds->getPropertyByName("WeaponWin")));
    }
  
    return attnum;
}

bool Combatant::hit(uint32_t firepower){
    damage += firepower;
    return isDead();
}

bool Combatant::isDead() const{
    if(shiptype == 0){
        return (damage >= 6);
    }
    DesignStore::Ptr ds = Game::getGame()->getDesignStore();
    
    propertyid_t armourprop = ds->getPropertyByName("Armour");
    if(armourprop == 0){
        armourprop = ds->getPropertyByName("Amour");
    }
    
    Design::Ptr design = ds->getDesign(shiptype);
    return (damage >= (uint32_t)design->getPropertyValue(armourprop));

}

uint32_t Combatant::getDamage() const{
    return damage;
}

void Combatant::setDamage(uint32_t nd){
    damage = nd;
}
