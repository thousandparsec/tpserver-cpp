/*  PlayerView object, holding the player's view of the universe
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

// #include "string.h"
#include <time.h>

#include "logging.h"
#include "game.h"
#include "designstore.h"
#include "persistence.h"

#include "playerview.h"

PlayerView::PlayerView() : pid(0){
}

PlayerView::~PlayerView(){
}

void PlayerView::setPlayerId(uint32_t newid){
  pid = newid;
  objects.pid = newid;
  designs.pid = newid;
  components.pid = newid;
}

void PlayerView::doOnceATurn(){
  objects.sequence++;
  designs.sequence++;
  components.sequence++;
}


void PlayerView::addVisibleObject(ObjectView::Ptr obj){
  objects.addVisible( obj );
}

void PlayerView::addVisibleObject( uint32_t id, bool completely_visible ){
  objects.addVisible( ObjectView::Ptr( new ObjectView( id, completely_visible ) ) );
}

void PlayerView::addVisibleObjects( const IdSet& obids ) {
  objects.addVisible( obids );
}


ObjectView::Ptr PlayerView::getObjectView(uint32_t objid){
  if(objects.isVisible(objid)){
    return objects.retrieve(objid);
  }else{
    return ObjectView::Ptr();
  }
}

void PlayerView::updateObjectView(uint32_t objid){
  ObjectView::Ptr obj = objects.cache[objid];
  Game::getGame()->getPersistence()->saveObjectView(pid, obj);
}

void PlayerView::removeVisibleObject(uint32_t objid){
  ObjectView::Ptr obj = getObjectView(objid);
  if (obj) {
    objects.cache[objid]->setGone(true);
    updateObjectView(objid);
  }
}

bool PlayerView::isVisibleObject(uint32_t objid){
  ObjectView::Ptr obj = getObjectView(objid);
  if ( obj ) return !(obj->isGone());
  return false;
}

IdSet PlayerView::getVisibleObjects() const{
  return objects.visible;
}

void PlayerView::addOwnedObject(uint32_t objid){
  objects.addActable( objid );
}

void PlayerView::removeOwnedObject(uint32_t objid){
  objects.removeActable(objid);
}

uint32_t PlayerView::getNumberOwnedObjects() const{
  return objects.actable.size();
}

IdSet PlayerView::getOwnedObjects() const{
  return objects.actable;
}

void PlayerView::processGetObjectIds(InputFrame::Ptr in, OutputFrame::Ptr out){
  objects.processGetIds( in, out, ft03_ObjectIds_List );
}

void PlayerView::addVisibleDesign(DesignView::Ptr design){
  designs.addVisible( design );
}

void PlayerView::addVisibleDesign( uint32_t id, bool completely_visible ){
  designs.addVisible( DesignView::Ptr( new DesignView( id, completely_visible ) ) );
}

void PlayerView::addVisibleDesigns( const IdSet& obids ) {
  designs.addVisible( obids );
}

void PlayerView::addUsableDesign(uint32_t designid){
  designs.addActable( designid );
}

void PlayerView::removeUsableDesign(uint32_t designid){
  designs.removeActable(designid);
}

bool PlayerView::isUsableDesign(uint32_t designid) const{
  return designs.isActable(designid);
}

IdSet PlayerView::getUsableDesigns() const{
  return designs.actable;
}

IdSet PlayerView::getVisibleDesigns() const{
  return designs.visible;
}

DesignView::Ptr PlayerView::getDesignView(uint32_t designid){
  if(!designs.isVisible(designid)){
    return DesignView::Ptr();
  }else{
    return designs.retrieve(designid);
  }
}

void PlayerView::processGetDesignIds(InputFrame::Ptr in, OutputFrame::Ptr out){
  designs.processGetIds( in, out, ft03_DesignIds_List );
}

void PlayerView::addVisibleComponent(ComponentView::Ptr comp){
  components.addVisible( comp );
}

void PlayerView::addVisibleComponents( const IdSet& obids ) {
  components.addVisible( obids );
}

void PlayerView::addUsableComponent(uint32_t compid){
  components.addActable( compid );
}

void PlayerView::removeUsableComponent(uint32_t compid){
  components.removeActable(compid);
}

bool PlayerView::isUsableComponent(uint32_t compid) const{
  return (components.actable.find(compid) != components.actable.end());
}

IdSet PlayerView::getVisibleComponents() const{
  return components.visible;
}

IdSet PlayerView::getUsableComponents() const{
  return components.actable;
}

ComponentView::Ptr PlayerView::getComponentView(uint32_t compid ){
  if(components.visible.find(compid) == components.visible.end()){
    return ComponentView::Ptr();
  }else{
    return components.retrieve(compid);
  }
}

void PlayerView::processGetComponentIds(InputFrame::Ptr in, OutputFrame::Ptr out){
  components.processGetIds( in, out, ft03_ComponentIds_List );
}

void PlayerView::setVisibleObjects(const IdSet& obids){
  objects.visible = obids;
}

void PlayerView::setOwnedObjects(const IdSet& obids){
  objects.actable = obids;
}

void PlayerView::setVisibleDesigns(const IdSet& dids){
  designs.visible = dids;
}

void PlayerView::setUsableDesigns(const IdSet& dids){
  designs.actable = dids;
}

void PlayerView::setVisibleComponents(const IdSet& cids){
  components.visible = cids;
}

void PlayerView::setUsableComponents(const IdSet& cids){
  components.actable = cids;
}

template< class EntityType, FrameType ft >
void PlayerView::EntityInfo< EntityType, ft >::processGetIds( InputFrame::Ptr in, OutputFrame::Ptr out, FrameType type )
{
  DEBUG("doing Get Ids frame");
  
  if(in->getVersion() < fv0_3){
    DEBUG("protocol version not high enough");
    throw FrameException(fec_FrameError, "Get ids isn't supported in this protocol");
  }
  
  if((in->getDataLength() != 12 && in->getVersion() <= fv0_3) || (in->getDataLength() != 20 && in->getVersion() >= fv0_4)){
    throw FrameException(fec_FrameError, "Invalid frame");
  }
  
  uint32_t seqnum = in->unpackInt();
  uint32_t snum = in->unpackInt();
  uint32_t numtoget = in->unpackInt();
  uint64_t fromtime = UINT64_NEG_ONE;
  if(in->getVersion() >= fv0_4){
    fromtime = in->unpackInt64();
  }
  
  if(seqnum != sequence && seqnum != UINT32_NEG_ONE){
    modified.clear();
    throw FrameException(fec_FrameError, "Invalid Sequence number");
  }
  
  if(seqnum == UINT32_NEG_ONE){
    generateModList( fromtime );
  }
  
  if(snum > modified.size()){
    DEBUG("Starting number too high, snum = %d, size = %d", snum, visible.size());
    throw FrameException(fec_NonExistant, "Starting number too high");
  }
  if(numtoget > modified.size() - snum){
    numtoget = modified.size() - snum;
  }
    
  if(numtoget > MAX_ID_LIST_SIZE + ((out->getVersion() < fv0_4)? 1 : 0)){
    DEBUG("Number of items to get too high, numtoget = %d", numtoget);
    throw FrameException(fec_FrameError, "Too many items to get, frame too big");
    return;
  }
    
  out->setType(type);
  out->packInt(sequence);
  out->packIdModList(modified,numtoget,snum);
  if(out->getVersion() >= fv0_4){
    out->packInt64(fromtime);
  }
}

template< class EntityType, FrameType ft >
void PlayerView::EntityInfo< EntityType, ft >::generateModList( uint64_t fromtime )
{
  modified.clear();
  for(IdSet::iterator itcurr = visible.begin();
      itcurr != visible.end(); ++itcurr){
    boost::shared_ptr< EntityType > viewobj = retrieve(*itcurr);
    uint64_t modtime = viewobj->getModTime();
    if((fromtime == UINT64_NEG_ONE && !viewobj->isGone()) || modtime > fromtime){
      modified[*itcurr] = modtime;
    }
  }
}


template< class EntityType, FrameType ft >
void PlayerView::EntityInfo< EntityType, ft >::addVisible( EntityPtr entity )
{
  uint32_t id = entity->getId();
  entity->touchModTime();
  Game::getGame()->getPersistence()->saveProtocolView(pid, entity);
  visible.insert(id);
  cache[id] = entity;
  sequence++;
}

template< class EntityType, FrameType ft >
void PlayerView::EntityInfo< EntityType, ft >::addVisible( const IdSet& obids ){
  for (IdSet::const_iterator it = obids.begin(); it != obids.end(); ++it ) {
    addVisible( EntityPtr( new EntityType( *it, true ) ) );
  }
}

template< class EntityType, FrameType ft >
void PlayerView::EntityInfo< EntityType, ft >::addActable( uint32_t id )
{
  actable.insert(id);
  if(visible.find(id) == visible.end()){
    addVisible( EntityPtr( new EntityType( id, true ) ) );
  }else{
    EntityPtr view = retrieve(id);
    view->setCompletelyVisible(true);
    Game::getGame()->getPersistence()->saveProtocolView(pid, view);
    sequence++;
  }
}


template< class EntityType, FrameType ft >
void PlayerView::EntityInfo< EntityType, ft >::removeActable( uint32_t id )
{
  IdSet::iterator f = actable.find(id);
  if(f != actable.end())
    actable.erase(f);
}

template< class EntityType, FrameType ft >
bool PlayerView::EntityInfo< EntityType, ft >::isActable( uint32_t id ) const
{
  return actable.find( id ) != actable.end();
}

template< class EntityType, FrameType ft >
bool PlayerView::EntityInfo< EntityType, ft >::isVisible( uint32_t id ) const
{
  return visible.find( id ) != visible.end();
}

template< class EntityType, FrameType ft >
boost::shared_ptr< EntityType > 
PlayerView::EntityInfo< EntityType, ft >::retrieve( uint32_t id ) 
{
  EntityPtr entity = cache[id];
  if(entity == NULL){
    entity = boost::dynamic_pointer_cast< EntityType >(Game::getGame()->getPersistence()->retrieveProtocolView(frame_type, pid, id));
    if(entity != NULL){
      cache[id] = entity;
    }
  }
  return entity;
}
