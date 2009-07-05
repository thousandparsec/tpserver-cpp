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

bool Persistence::saveObject(IGObject* ob){
    return false;
}

IGObject* Persistence::retrieveObject(uint32_t obid){
    return NULL;
}

uint32_t Persistence::getMaxObjectId(){
    return 0;
}

IdSet Persistence::getObjectIds(){
    return IdSet();
}

bool Persistence::saveOrderQueue(const OrderQueue* oq){
  return false;
}

bool Persistence::updateOrderQueue(const OrderQueue* oq){
  return false;
}

OrderQueue* Persistence::retrieveOrderQueue(uint32_t oqid){
  return NULL;
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

bool Persistence::updateBoard(const Board* board){
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

bool Persistence::saveMessage(uint32_t msgid, Message::Ptr msg){
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

bool Persistence::saveResource(ResourceDescription* res){
    return false;
}

ResourceDescription* Persistence::retrieveResource(uint32_t restype){
    return NULL;
}

uint32_t Persistence::getMaxResourceId(){
    return 0;
}

IdSet Persistence::getResourceIds(){
    return IdSet();
}

bool Persistence::savePlayer(Player* player){
    return false;
}

bool Persistence::updatePlayer(Player* player){
    return false;
}

Player* Persistence::retrievePlayer(uint32_t playerid){
    return NULL;
}

uint32_t Persistence::getMaxPlayerId(){
    return 0;
}

IdSet Persistence::getPlayerIds(){
    return IdSet();
}

bool Persistence::saveCategory(Category* cat){
    return false;
}

Category* Persistence::retrieveCategory(uint32_t catid){
    return NULL;
}

uint32_t Persistence::getMaxCategoryId(){
    return 0;
}

IdSet Persistence::getCategoryIds(){
    return IdSet();
}

bool Persistence::saveDesign(Design* design){
    return false;
}

bool Persistence::updateDesign(Design* design){
    return false;
}

Design* Persistence::retrieveDesign(uint32_t designid){
    return NULL;
}

uint32_t Persistence::getMaxDesignId(){
    return 0;
}

IdSet Persistence::getDesignIds(){
    return IdSet();
}

bool Persistence::saveComponent(Component* comp){
    return false;
}

Component* Persistence::retrieveComponent(uint32_t compid){
    return NULL;
}

uint32_t Persistence::getMaxComponentId(){
    return 0;
}

IdSet Persistence::getComponentIds(){
    return IdSet();
}

bool Persistence::saveProperty(Property* prop){
    return false;
}

Property* Persistence::retrieveProperty(uint32_t propid){
    return NULL;
}

uint32_t Persistence::getMaxPropertyId(){
    return 0;
}

IdSet Persistence::getPropertyIds(){
    return IdSet();
}

bool Persistence::saveObjectView(uint32_t playerid, ObjectView*){
  return false;
}

ObjectView* Persistence::retrieveObjectView(uint32_t playerid, uint32_t objectid, uint32_t turn){
  return NULL;
}

bool Persistence::saveDesignView(uint32_t playerid, DesignView*){
  return false;
}

DesignView* Persistence::retrieveDesignView(uint32_t playerid, uint32_t designid){
  return NULL;
}

bool Persistence::saveComponentView(uint32_t playerid, ComponentView*){
  return false;
}

ComponentView* Persistence::retrieveComponentView(uint32_t playerid, uint32_t componentid){
  return NULL;
}

bool Persistence::saveProtocolView(uint32_t playerid, ProtocolView* view)
{
  switch(view->getFrameType()) {
    case ft03_Component : return saveComponentView( playerid, dynamic_cast<ComponentView*>(view)); 
    case ft03_Design : return saveDesignView( playerid, dynamic_cast<DesignView*>(view)); 
    case ft02_Object : return saveObjectView( playerid, dynamic_cast<ObjectView*>(view)); 
    default : return false;
  }
}

ProtocolView* Persistence::retrieveProtocolView(FrameType viewtype, uint32_t playerid, uint32_t objectid)
{
  switch(viewtype) {
    case ft03_Component : return retrieveComponentView( playerid, objectid );
    case ft03_Design : return retrieveDesignView( playerid, objectid );
    case ft02_Object : return retrieveObjectView( playerid, objectid );
    default : return NULL;
  }
}
