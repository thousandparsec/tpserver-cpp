/*  Dummy Fleet
 *
 *  Copyright (C) 2009 Alan P. Laudicina and the Thousand Parsec Project
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
#include <tpserver/logging.h>

#include "fleet.h"
#include "dummyfleet.h"

namespace MTSecRuleset {

DummyFleet::DummyFleet(): playerref(0), shiplist(), damage(0){
    Logger::getLogger()->debug("New dummyfleet");
}

DummyFleet::~DummyFleet(){
}

uint32_t DummyFleet::getOwner() const{
  return playerref;
}

void DummyFleet::setOwner(uint32_t no){
  playerref = no;
}

void DummyFleet::addShips(uint32_t type, uint32_t number){
  Logger::getLogger()->debug("Adding ships %d : %d", type, number);
  shiplist[type] += number;
}

bool DummyFleet::removeShips(uint32_t type, uint32_t number){
  if(shiplist[type] >= number){
    shiplist[type] -= number;
    return true;
  }
  return false;
}

uint32_t DummyFleet::numShips(uint32_t type){
  return shiplist[type];
}

std::map<uint32_t, uint32_t> DummyFleet::getShips() const{
  return shiplist;
}

uint32_t DummyFleet::totalShips() const{
  uint32_t num = 0;
  for(std::map<uint32_t, uint32_t>::const_iterator itcurr = shiplist.begin();
      itcurr != shiplist.end(); ++itcurr){
    num += itcurr->second;
  }
  return num;
}

uint32_t DummyFleet::getDamage() const{
    return damage;
}

void DummyFleet::setDamage(uint32_t nd){
    damage = nd;
}

std::map<uint32_t, std::pair<uint32_t, uint32_t> > DummyFleet::getResources() {
    return std::map<uint32_t, std::pair<uint32_t, uint32_t> >();
}

bool DummyFleet::removeResource(uint32_t restype, uint32_t amount) {
    return false;
}

}
