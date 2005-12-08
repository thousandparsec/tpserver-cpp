/*  Persistence Interface
 * All methods return false or NULL, except init().
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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
#include "order.h"
#include "board.h"
#include "message.h"
#include "player.h"
#include "category.h"
#include "design.h"
#include "component.h"
#include "property.h"

#include "persistence.h"

Persistence::~Persistence(){
}

bool Persistence::init(){
    return true;
}

void Persistence::shutdown(){
}

bool Persistence::saveObject(IGObject* ob){
    return false;
}

bool Persistence::updateObject(IGObject* ob){
    return false;
}

IGObject* Persistence::retrieveObject(uint32_t obid){
    return NULL;
}

bool Persistence::removeObject(uint32_t obid){
    return false;
}

uint32_t Persistence::getMaxObjectId(){
    return 0;
}

std::set<uint32_t> Persistence::getObjectIds(){
    return std::set<uint32_t>();
}

bool Persistence::saveOrder(uint32_t ordid, Order* ord){
    return false;
}

bool Persistence::updateOrder(uint32_t ordid, Order* ord){
    return false;
}

Order* Persistence::retrieveOrder(uint32_t ordid){
    return NULL;
}

bool Persistence::removeOrder(uint32_t ordid){
    return false;
}

bool Persistence::saveOrderList(uint32_t obid, std::list<uint32_t> list){
    return false;
}

std::list<uint32_t> Persistence::retrieveOrderList(uint32_t obid){
    return std::list<uint32_t>();
}

std::set<uint32_t> Persistence::retrieveObjectsWithOrders(){
    return std::set<uint32_t>();
}

uint32_t Persistence::getMaxOrderId(){
    return 0;
}

bool Persistence::saveBoard(Board* board){
    return false;
}

Board* Persistence::retrieveBoard(uint32_t boardid){
    return NULL;
}

bool Persistence::saveMessage(Message* msg){
    return false;
}

Message* Persistence::retrieveMessage(uint32_t msgpid){
    return NULL;
}

Message* Persistence::retrieveMessage(uint32_t boardid, uint32_t slot){
    return NULL;
}

bool Persistence::savePlayer(Player* player){
    return false;
}

Player* Persistence::retrievePlayer(uint32_t playerid){
    return NULL;
}

Player* Persistence::retrievePlayer(const std::string& name){
    return NULL;
}

bool Persistence::saveCategory(Category* cat){
    return false;
}

Category* Persistence::retriveCategory(uint32_t catid){
    return NULL;
}

bool Persistence::saveDesign(Design* design){
    return false;
}

Design* Persistence::retrieveDesign(uint32_t designid){
    return NULL;
}

bool Persistence::saveComponent(Component* comp){
    return false;
}

Component* Persistence::retrieveComponent(uint32_t compid){
    return NULL;
}

bool Persistence::saveProperty(Property* prop){
    return false;
}

Property* Persistence::retrieveProperty(uint32_t propid){
    return NULL;
}

