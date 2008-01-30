/*  ObjectRelationships class
 *
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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

#include "frame.h"
#include "player.h"
#include "playermanager.h"
#include "playerview.h"
#include "game.h"

#include "objectrelationships.h"

ObjectRelationshipsData::ObjectRelationshipsData() : ref(0), parentid(0), children(), dirty(true){
}

ObjectRelationshipsData::~ObjectRelationshipsData(){
}

uint32_t ObjectRelationshipsData::getParent() const{
  return parentid;
}

std::set<uint32_t> ObjectRelationshipsData::getChildren() const{
  return children;
}

void ObjectRelationshipsData::setParent(uint32_t np){
  dirty = (np != parentid);
  parentid = np;
}

void ObjectRelationshipsData::addChild(uint32_t nc){
  dirty = children.insert(nc).second;
}

void ObjectRelationshipsData::removeChild(uint32_t oc){
  dirty = (children.erase(oc) != 0);
}

void ObjectRelationshipsData::setChildren(const std::set<uint32_t> nc){
  children = nc;
  dirty = true;
}

void ObjectRelationshipsData::packFrame(Frame* frame, uint32_t playerid){
 
 
  std::set<uint32_t> temp = children;
  std::set <uint32_t>::iterator itcurr, itend;
  itcurr = temp.begin();
  itend = temp.end();
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(playerid);
  while(itcurr != itend){
    if(!player->getPlayerView()->isVisibleObject(*itcurr)){
      std::set<unsigned int>::iterator itemp = itcurr;
      ++itcurr;
      temp.erase(itemp);
    }else{
      ++itcurr;
    }
  }

  frame->packInt(temp.size());
  //for loop for children objects
  itend = temp.end();
  for (itcurr = temp.begin(); itcurr != itend; itcurr++) {
    frame->packInt(*itcurr);
  }
}

void ObjectRelationshipsData::unpackModFrame(Frame* f){
  //discard all data
  f->unpackInt();
  uint32_t numchildren = f->unpackInt();
  for(uint32_t i = 0; i < numchildren; i++){
    f->unpackInt();
  }
}

bool ObjectRelationshipsData::isDirty() const{
  return dirty;
}

void ObjectRelationshipsData::setIsDirty(bool id){
  dirty = id;
}
