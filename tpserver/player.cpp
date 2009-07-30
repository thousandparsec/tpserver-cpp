/*  Player object, processing and frame handling
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include "frame.h"
#include "logging.h"
#include "game.h"
#include "board.h"
#include "message.h"
#include "boardmanager.h"
#include "playerview.h"

#include "player.h"


Player::Player() : ProtocolObject(ft03_Player,0,"",""), passwd(), email(), boardid(0), 
                alive(true), score(){
  playerview.reset( new PlayerView() );
}

Player::~Player(){
}

void Player::setName(const std::string& newname){
  Describable::setName( newname );
  touchModTime();
}

void Player::setPass(const std::string& newpass){
    passwd = newpass;
    touchModTime();
}

void Player::setEmail(const std::string& newemail){
  email = newemail;
  touchModTime();
}

void Player::setComment(const std::string& newcomm){
  Describable::setDescription( newcomm );
  touchModTime();
}

void Player::setId(uint32_t newid){
  Describable::setId( newid );
  playerview->setPlayerId(newid);
  touchModTime();
}

void Player::setIsAlive(bool na){
  alive = na;
  touchModTime();
}

void Player::setScore(uint32_t key, uint32_t value){
  score[key] = value;
  touchModTime();
}

void Player::postToBoard(Message* msg){
  Game::getGame()->getBoardManager()->postToBoard(Message::Ptr(msg),boardid);
}

std::string Player::getPass() const{
  return passwd;
}

std::string Player::getEmail() const{
  return email;
}

std::string Player::getComment() const{
  return getDescription();
}

uint32_t Player::getID() const{
  return getId();
}

uint32_t Player::getBoardId() const{
  return boardid;
}

bool Player::isAlive() const{
  return alive;
}

uint32_t Player::getScore(uint32_t key) const{
  IdMap::const_iterator itval = score.find(key);
  if(itval != score.end()){
    return itval->second;
  }else{
    return 0;
  }
}

IdMap Player::getAllScores() const{
  return score;
}

void Player::setBoardId(uint32_t nbi){
  boardid = nbi;
  touchModTime();
}

PlayerView::Ptr Player::getPlayerView() const{
  return playerview;
}

void Player::pack(Frame* frame) const {
  frame->setType(ft03_Player);
  frame->packInt(id);
  frame->packString(name);
  frame->packString("Human");
}

