/*  Player object, processing and frame handling
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include "frame.h"
#include "logging.h"
#include "game.h"
#include "board.h"
#include "message.h"
#include "boardmanager.h"
#include "designstore.h"
#include "design.h"
#include "component.h"

#include "player.h"


Player::Player(){
  pid = 0;
  currObjSeq = 0;
  boardid = 0;
}

Player::~Player(){
}

void Player::setName(const std::string& newname){
    name = newname;
}

void Player::setPass(const std::string& newpass){
    passwd = newpass;
}

void Player::setId(uint32_t newid){
  pid = newid;
}

void Player::setVisibleObjects(std::set<unsigned int> vis){
  visibleObjects = vis;
  currObjSeq++;
}

bool Player::isVisibleObject(unsigned int objid){
  return visibleObjects.find(objid) != visibleObjects.end();
}

std::set<uint32_t> Player::getVisibleObjects() const{
  return visibleObjects;
}

void Player::addVisibleDesign(unsigned int designid){
  visibleDesigns.insert(designid);
}

void Player::addUsableDesign(unsigned int designid){
  usableDesigns.insert(designid);
  Logger::getLogger()->debug("Added valid design");
}

void Player::removeUsableDesign(unsigned int designid){
  std::set<unsigned int>::iterator dicurr = usableDesigns.find(designid);
  if(dicurr != usableDesigns.end())
    usableDesigns.erase(dicurr);
}

bool Player::isUsableDesign(unsigned int designid) const{
  return (usableDesigns.find(designid) != usableDesigns.end());
}

std::set<unsigned int> Player::getUsableDesigns() const{
  return usableDesigns;
}

std::set<uint32_t> Player::getVisibleDesigns() const{
  return visibleDesigns;
}

void Player::addVisibleComponent(unsigned int compid){
  visibleComponents.insert(compid);
}

void Player::addUsableComponent(unsigned int compid){
  usableComponents.insert(compid);
}

void Player::removeUsableComponent(unsigned int compid){
  std::set<unsigned int>::iterator cicurr = usableComponents.find(compid);
  if(cicurr != usableComponents.end())
    usableComponents.erase(cicurr);
}

bool Player::isUsableComponent(unsigned int compid){
  return (usableComponents.find(compid) != usableComponents.end());
}

std::set<uint32_t> Player::getVisibleComponents() const{
  return visibleComponents;
}

std::set<uint32_t> Player::getUsableComponents() const{
  return usableComponents;
}

void Player::postToBoard(Message* msg){
  Board* board = Game::getGame()->getBoardManager()->getBoard(boardid);
  board->addMessage(msg, -1);
}

std::string Player::getName() const{
  return name;
}

std::string Player::getPass() const{
  return passwd;
}

uint32_t Player::getID(){
  return pid;
}

uint32_t Player::getBoardId() const{
  return boardid;
}

void Player::setBoardId(uint32_t nbi){
  boardid = nbi;
}

uint32_t Player::getObjectSequenceKey() const{
  return currObjSeq;
}

void Player::packFrame(Frame* frame){
  frame->setType(ft03_Player);
  frame->packInt(pid);
  frame->packString(name.c_str());
  frame->packString("Human");
}
