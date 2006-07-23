/*  Fleet object
 *
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

#include <math.h>

#include <tpserver/frame.h>
#include <tpserver/order.h>
#include <tpserver/game.h>
#include "planet.h"
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objectdatamanager.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/logging.h>

#include "fleet.h"

Fleet::Fleet():OwnedObject()
{
    damage = 0;
}

Fleet::~Fleet(){
}

void Fleet::addShips(int type, int number){
  ships[type] += number;
  touchModTime();
}

bool Fleet::removeShips(int type, int number){
  if(ships[type] >= number){
    ships[type] -= number;
    touchModTime();
    return true;
  }
  return false;
}

int Fleet::numShips(int type){
  return ships[type];
}

std::map<int, int> Fleet::getShips() const{
  return ships;
}

int Fleet::totalShips() const{
  int num = 0;
  for(std::map<int, int>::const_iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
    num += itcurr->second;
  }
  return num;
}

long long Fleet::maxSpeed(){
  double speed = 1e100;
  DesignStore* ds = Game::getGame()->getDesignStore();
  for(std::map<int, int>::iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
        speed = fmin(speed, ds->getDesign(itcurr->first)->getPropertyValue(ds->getPropertyByName("Speed")));
  }
  return (long long)(floor(speed));
}


// This routine returns the amount of damage this fleet does when
// attacking another fleet, in units of hit points.
//
// This is simply a summation of the damage that each ship in
// the fleet can do, as indicated by the firepower
// property value in the ship's design.
unsigned int Fleet::firepower( bool draw) {
  double fp = 0;
  DesignStore* ds = Game::getGame()->getDesignStore();

  for ( std::map<int, int>::iterator itcurr = ships.begin();
        itcurr != ships.end(); ++itcurr) {
    if ( draw) {
      fp += ds->getDesign(itcurr->first)->getPropertyValue(ds->getPropertyByName("WeaponDraw")) * itcurr->second;
    }else{
      fp += ds->getDesign(itcurr->first)->getPropertyValue(ds->getPropertyByName("WeaponWin")) * itcurr->second;
    }
  }
  return (int) (floor(fp));
}


// Return the design ID of the ship design in the fleet
// that has the most hit points
unsigned int Fleet::getLargestShipType()
{
    DesignStore* ds = Game::getGame()->getDesignStore();
    unsigned int shiptype = 0;
    unsigned int shiphp = 0;

    for ( std::map<int, int>::iterator itcurr = ships.begin();
          itcurr != ships.end(); ++itcurr) {
        Design *design = ds->getDesign( itcurr->first);
        if ( shiphp < ( unsigned int) design->getPropertyValue( ds->getPropertyByName("Amour")) &&
             itcurr->second > 0) {
            shiptype = itcurr->first;
            shiphp = ( int) design->getPropertyValue( ds->getPropertyByName("Amour"));
        }
    }

    return shiptype;
}


// Destroy one ship of the given design.
void Fleet::shipDestroyed( unsigned int designID)
{
    DesignStore* ds = Game::getGame()->getDesignStore();
    Design* design = ds->getDesign(designID);

    ships[designID]--;
    design->removeDestroyed( 1);
    ds->designCountsUpdated( design);
    if ( ships[designID] == 0) {
        ships.erase( designID);
    }

    return;
}


// The fleet was damaged.  Damage done to this fleet is given
// in firepower, in units of hit points.  This is added to any
// existing damage (from previous hits), then any effects on
// the fleet are determined.  If the cumulative damage is greater
// than the hit points of the largest ship, that ship is considered
// destroyed.
//
// The return value from this routine is true if there are any
// ships left in the fleet after the damage is taken into account,
// false otherwise.
bool Fleet::hit( unsigned int firepower)
{
    if ( firepower != 0) {
        DesignStore* ds = Game::getGame()->getDesignStore();
        unsigned int shiptype = getLargestShipType();

        if ( shiptype != 0) {
            Design *design = ds->getDesign( shiptype);
            unsigned int armor = floor( design->getPropertyValue( ds->getPropertyByName("Amour")));

            // If the ship has armor, use it
            if ( firepower > armor) {
                firepower -= armor;
                damage += firepower;

                if ( damage > design->getPropertyValue( ds->getPropertyByName("HitPoints"))) {
                    shipDestroyed( shiptype);
                    damage = 0;
                }
            }
            else {
                // firepower expended itself on the armor, no damage
            }
        }

        touchModTime();
    }

    return ( ships.size() > 0);
}


int Fleet::getDamage() const{
    return damage;
}

void Fleet::setDamage(int nd){
    damage = nd;
    touchModTime();
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
  IGObject * pob = Game::getGame()->getObjectManager()->getObject(obj->getParent());
  if(pob->getType() == obT_Planet && ((Planet*)(pob->getObjectData()))->getOwner() == getOwner()){
    if(damage != 0){
        damage = 0;
        touchModTime();
    }
  }
    Game::getGame()->getObjectManager()->doneWithObject(obj->getParent());
}

void Fleet::packAllowedOrders(Frame * frame, int playerid){
  if(playerid == getOwner()){
    bool colonise = false;
    DesignStore* ds = Game::getGame()->getDesignStore();
    for(std::map<int, int>::iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
        if(ds->getDesign(itcurr->first)->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.0){
	colonise = true;
	break;
      }
    }
    if(colonise){
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
  bool colonise = false;
  DesignStore* ds = Game::getGame()->getDesignStore();
  for(std::map<int, int>::iterator itcurr = ships.begin();
      itcurr != ships.end(); ++itcurr){
        if(ds->getDesign(itcurr->first)->getPropertyValue(ds->getPropertyByName("Colonise")) == 1.0){
      colonise = true;
      break;
    }
  }
  return (playerid == getOwner() && (ot == odT_Move || ot == odT_Nop || ot == odT_Fleet_Split || ot == odT_Fleet_Merge || (colonise && ot == odT_Colonise)));
}

int Fleet::getContainerType(){
  return 0;
}

ObjectData* Fleet::clone(){
  return new Fleet();
}
