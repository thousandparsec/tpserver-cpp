/*  Persistence Interface
 * All methods return false or NULL, except init().
 *
 *  Copyright (C) 2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include "orderqueue.h"
#include "order.h"
#include "board.h"
#include "message.h"
#include "resourcedescription.h"
#include "player.h"
#include "category.h"
#include "design.h"
#include "component.h"
#include "property.h"
#include "objectview.h"
#include "designview.h"
#include "componentview.h"

#include "persistence.h"

Persistence::~Persistence(){
}

bool Persistence::init(){
    return true;
}

void Persistence::shutdown(){
}

bool Persistence::saveGameInfo(){
  return false;
}

bool Persistence::retrieveGameInfo(){
  return false;
}

bool Persistence::saveObject(IGObject::Ptr ob){
    return false;
}

IGObject::Ptr Persistence::retrieveObject(uint32_t obid){
    return IGObject::Ptr();
}

uint32_t Persistence::getMaxObjectId(){
    return 0;
}

IdSet Persistence::getObjectIds(){
    return IdSet();
}

bool Persistence::saveOrderQueue(const OrderQueue::Ptr oq){
  return false;
}

bool Persistence::updateOrderQueue(const OrderQueue::Ptr oq){
  return false;
}

OrderQueue::Ptr Persistence::retrieveOrderQueue(uint32_t oqid){
  return OrderQueue::Ptr();
}

bool Persistence::removeOrderQueue(uint32_t oqid){
  return false;
}

IdSet Persistence::getOrderQueueIds(){
  return IdSet();
}

uint32_t Persistence::getMaxOrderQueueId(){
  return 0;
}

bool Persistence::saveOrder(uint32_t queueid, uint32_t ordid, Order* ord){
    return false;
}

bool Persistence::updateOrder(uint32_t queueid, uint32_t ordid, Order* ord){
    return false;
}

Order* Persistence::retrieveOrder(uint32_t queueid, uint32_t ordid){
    return NULL;
}

bool Persistence::removeOrder(uint32_t queueid, uint32_t ordid){
    return false;
}


bool Persistence::saveBoard(boost::shared_ptr<Board> board){
    return false;
}

bool Persistence::updateBoard(boost::shared_ptr<Board> board){
    return false;
}

boost::shared_ptr<Board> Persistence::retrieveBoard(uint32_t boardid){
    return boost::shared_ptr<Board>();
}

uint32_t Persistence::getMaxBoardId(){
    return 0;
}

IdSet Persistence::getBoardIds(){
    return IdSet();
}

bool Persistence::saveMessage( Message::Ptr msg){
    return false;
}

boost::shared_ptr< Message > Persistence::retrieveMessage(uint32_t msgid){
    return boost::shared_ptr< Message >();
}

bool Persistence::removeMessage(uint32_t msgid){
    return false;
}

bool Persistence::saveMessageList(uint32_t bid, std::list<uint32_t> list){
    return false;
}

std::list<uint32_t> Persistence::retrieveMessageList(uint32_t bid){
    return std::list<uint32_t>();
}

uint32_t Persistence::getMaxMessageId(){
    return 0;
}

bool Persistence::saveResource(ResourceDescription::Ptr res){
    return false;
}

ResourceDescription::Ptr Persistence::retrieveResource(uint32_t restype){
    return ResourceDescription::Ptr();
}

uint32_t Persistence::getMaxResourceId(){
    return 0;
}

IdSet Persistence::getResourceIds(){
    return IdSet();
}

bool Persistence::savePlayer(Player::Ptr player){
    return false;
}

bool Persistence::updatePlayer(Player::Ptr player){
    return false;
}

Player::Ptr Persistence::retrievePlayer(uint32_t playerid){
    return Player::Ptr();
}

uint32_t Persistence::getMaxPlayerId(){
    return 0;
}

IdSet Persistence::getPlayerIds(){
    return IdSet();
}

bool Persistence::saveCategory(Category::Ptr cat){
    return false;
}

Category::Ptr Persistence::retrieveCategory(uint32_t catid){
    return Category::Ptr();
}

uint32_t Persistence::getMaxCategoryId(){
    return 0;
}

IdSet Persistence::getCategoryIds(){
    return IdSet();
}

bool Persistence::saveDesign(Design::Ptr design){
    return false;
}

bool Persistence::updateDesign(Design::Ptr design){
    return false;
}

Design::Ptr Persistence::retrieveDesign(uint32_t designid){
    return Design::Ptr();
}

uint32_t Persistence::getMaxDesignId(){
    return 0;
}

IdSet Persistence::getDesignIds(){
    return IdSet();
}

bool Persistence::saveComponent(Component::Ptr comp){
    return false;
}

Component::Ptr Persistence::retrieveComponent(uint32_t compid){
    return Component::Ptr();
}

uint32_t Persistence::getMaxComponentId(){
    return 0;
}

IdSet Persistence::getComponentIds(){
    return IdSet();
}

bool Persistence::saveProperty(Property::Ptr prop){
    return false;
}

Property::Ptr Persistence::retrieveProperty(uint32_t propid){
    return Property::Ptr();
}

uint32_t Persistence::getMaxPropertyId(){
    return 0;
}

IdSet Persistence::getPropertyIds(){
    return IdSet();
}

bool Persistence::saveObjectView(uint32_t playerid, ObjectView::Ptr){
  return false;
}

ObjectView::Ptr Persistence::retrieveObjectView(uint32_t playerid, uint32_t objectid, uint32_t turn){
  return ObjectView::Ptr();
}

bool Persistence::saveDesignView(uint32_t playerid, DesignView::Ptr){
  return false;
}

DesignView::Ptr Persistence::retrieveDesignView(uint32_t playerid, uint32_t designid){
  return DesignView::Ptr();
}

bool Persistence::saveComponentView(uint32_t playerid, ComponentView::Ptr){
  return false;
}

ComponentView::Ptr Persistence::retrieveComponentView(uint32_t playerid, uint32_t componentid){
  return ComponentView::Ptr();
}

bool Persistence::saveProtocolView(uint32_t playerid, ProtocolView::Ptr view)
{
  switch(view->getFrameType()) {
    case ft03_Component : return saveComponentView( playerid, boost::dynamic_pointer_cast<ComponentView>(view)); 
    case ft03_Design : return saveDesignView( playerid, boost::dynamic_pointer_cast<DesignView>(view)); 
    case ft02_Object : return saveObjectView( playerid, boost::dynamic_pointer_cast<ObjectView>(view)); 
    default : return false;
  }
}

ProtocolView::Ptr Persistence::retrieveProtocolView(FrameType viewtype, uint32_t playerid, uint32_t objectid)
{
  switch(viewtype) {
    case ft03_Component : return retrieveComponentView( playerid, objectid );
    case ft03_Design : return retrieveDesignView( playerid, objectid );
    case ft02_Object : return retrieveObjectView( playerid, objectid );
    default : return ProtocolView::Ptr();
  }
}

bool Persistence::saveProtocolObject(ProtocolObject::Ptr object) 
{
  switch(object->getFrameType()) {
    case ft03_Component : return saveComponent( boost::dynamic_pointer_cast<Component>(object) );
    case ft03_Design    : return saveDesign( boost::dynamic_pointer_cast<Design>(object) );
    case ft03_Property  : return saveProperty( boost::dynamic_pointer_cast<Property>(object) );
    case ft03_Category  : return saveCategory( boost::dynamic_pointer_cast<Category>(object) );
    case ft03_Player    : return savePlayer( boost::dynamic_pointer_cast<Player>(object) );
    case ft02_ResDesc   : return saveResource( boost::dynamic_pointer_cast<ResourceDescription>(object) );
    case ft02_Object    : return saveObject( boost::dynamic_pointer_cast<IGObject>(object) );
    case ft02_Board     : return saveBoard( boost::dynamic_pointer_cast<Board>(object) );
    case ft02_Message   : return saveMessage( boost::dynamic_pointer_cast<Message>(object) );
    default : return false;
  }
}

ProtocolObject::Ptr Persistence::retrieveProtocolObject(FrameType objtype, uint32_t id )
{
  switch(objtype) {
    case ft03_Component : return retrieveComponent( id );
    case ft03_Design    : return retrieveDesign( id );
    case ft03_Property  : return retrieveProperty( id );
    case ft03_Category  : return retrieveCategory( id );
    case ft03_Player    : return retrievePlayer( id );
    case ft02_ResDesc   : return retrieveResource( id );
    case ft02_Object    : return retrieveObject( id );
    case ft02_Board     : return retrieveBoard( id );
    case ft02_Message   : return retrieveMessage( id );
    default : return ProtocolObject::Ptr();
  }
}

uint32_t Persistence::getMaxProtocolObjectId(FrameType objtype)
{
  switch(objtype) {
    case ft03_Component : return getMaxComponentId( );
    case ft03_Design    : return getMaxDesignId( );
    case ft03_Property  : return getMaxPropertyId( );
    case ft03_Category  : return getMaxCategoryId( );
    case ft03_Player    : return getMaxPlayerId( );
    case ft02_ResDesc   : return getMaxResourceId( );
    case ft02_Object    : return getMaxObjectId( );
    case ft02_Board     : return getMaxBoardId( );
    case ft02_Message   : return getMaxMessageId( );
    default : return 0;
  }
}


IdSet Persistence::getProtocolObjectIds(FrameType objtype)
{
  switch(objtype) {
    case ft03_Component : return getComponentIds( );
    case ft03_Design    : return getDesignIds( );
    case ft03_Property  : return getPropertyIds( );
    case ft03_Category  : return getCategoryIds( );
    case ft03_Player    : return getPlayerIds( );
    case ft02_ResDesc   : return getResourceIds( );
    case ft02_Object    : return getObjectIds( );
    case ft02_Board     : return getBoardIds( );
 // case ft02_Message   : return getMessageIds( );
    default : return IdSet();
  }
}


