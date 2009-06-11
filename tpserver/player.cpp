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


Player::Player() : name(), passwd(), email(), comment(), pid(0), boardid(0), 
               modtime(0), alive(true), score(){
  playerview = new PlayerView();
}

Player::~Player(){
  delete playerview;
}

void Player::setName(const std::string& newname){
    name = newname;
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
  comment = newcomm;
  touchModTime();
}

void Player::setId(uint32_t newid){
  pid = newid;
  playerview->setPlayerId(pid);
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
  Game::getGame()->getBoardManager()->postToBoard(msg,boardid);
}

std::string Player::getName() const{
  return name;
}

std::string Player::getPass() const{
  return passwd;
}

std::string Player::getEmail() const{
  return email;
}

std::string Player::getComment() const{
  return comment;
}

uint32_t Player::getID() const{
  return pid;
}

uint32_t Player::getBoardId() const{
  return boardid;
}

bool Player::isAlive() const{
  return alive;
}

uint32_t Player::getScore(uint32_t key) const{
  std::map<uint32_t,uint32_t>::const_iterator itval = score.find(key);
  if(itval != score.end()){
    return itval->second;
  }else{
    return 0;
  }
}

std::map<uint32_t, uint32_t> Player::getAllScores() const{
  return score;
}

uint64_t Player::getModTime() const{
  return modtime;
}

void Player::setBoardId(uint32_t nbi){
  boardid = nbi;
  touchModTime();
}

PlayerView* Player::getPlayerView() const{
  return playerview;
}

void Player::packFrame(Frame* frame){
  frame->setType(ft03_Player);
  frame->packInt(pid);
  frame->packString(name.c_str());
  frame->packString("Human");
}

void Player::setModTime(uint64_t nmt){
  modtime = nmt;
}

void Player::touchModTime(){
  modtime = time(NULL);
}
