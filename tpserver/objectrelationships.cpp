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
#include <boost/bind.hpp>

ObjectRelationshipsData::ObjectRelationshipsData() : parentid(0), children() {
}

ObjectRelationshipsData::~ObjectRelationshipsData(){
}

uint32_t ObjectRelationshipsData::getParent() const{
  return parentid;
}

IdSet ObjectRelationshipsData::getChildren() const{
  return children;
}

void ObjectRelationshipsData::setParent(uint32_t np){
  setIsDirty( (np != parentid) );
  parentid = np;
}

void ObjectRelationshipsData::addChild(uint32_t nc){
  setIsDirty( children.insert(nc).second );
}

void ObjectRelationshipsData::removeChild(uint32_t oc){
  setIsDirty( (children.erase(oc) != 0) );
}

void ObjectRelationshipsData::setChildren(const IdSet nc){
  children = nc;
  setIsDirty( true );
}

void ObjectRelationshipsData::packFrame(Frame* frame, uint32_t playerid){
  PlayerView::Ptr pv = Game::getGame()->getPlayerManager()->getPlayer(playerid)->getPlayerView();
  IdSet temp;
  std::remove_copy_if( children.begin(), children.end(), 
                       std::inserter( temp, temp.end() ), 
                       !boost::bind( &PlayerView::isVisibleObject, pv, _1 ) );
  frame->packIdSet(temp);
}

void ObjectRelationshipsData::unpackModFrame(Frame* f){
  //discard all data
  f->unpackInt();
  f->unpackIdSet();
}
