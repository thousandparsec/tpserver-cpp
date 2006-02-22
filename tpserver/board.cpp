/*  Messages boards
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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
#include "message.h"
#include "game.h"
#include "boardmanager.h"
#include "logging.h"

#include "board.h"

Board::Board(){
  modtime = time(NULL);
    nummsg = 0;
}

Board::~Board(){
}

void Board::setBoardID(int i){
  boardid = i;
  modtime = time(NULL);
}

int Board::getBoardID(){
  return boardid;
}

void Board::setName(const std::string & nname){
  name = nname;
  modtime = time(NULL);
}

std::string Board::getName(){
  return name;
}

void Board::setDescription(const std::string & ndest){
  description = ndest;
  modtime = time(NULL);
}

std::string Board::getDescription(){
  return description;
}

void Board::addMessage(Message * msg, int pos){
    bool success = false;
  if(pos == -1){
        if(Game::getGame()->getBoardManager()->addMessage(msg, this, nummsg)){
            success = true;
        }
  }else{
        if(Game::getGame()->getBoardManager()->addMessage(msg, this, pos)){
            success = true;
        }
  }
    if(success){
        modtime = time(NULL);
        Game::getGame()->getBoardManager()->updateBoard(boardid);
    }
}

bool Board::removeMessage(unsigned int pos){
  if(pos >= nummsg || pos < 0){
    return false;
  }
    Game::getGame()->getBoardManager()->removeMessage(this, pos);
  modtime = time(NULL);
    Game::getGame()->getBoardManager()->updateBoard(boardid);
  return true;
}

void Board::packBoard(Frame * frame){
  frame->setType(ft02_Board);
  frame->packInt(boardid);
  frame->packString(name.c_str());
  frame->packString(description.c_str());
  frame->packInt(nummsg);
  frame->packInt64(modtime);
}

void Board::packMessage(Frame * frame, unsigned int msgnum){
    if(msgnum < nummsg){
        Message* message = Game::getGame()->getBoardManager()->getMessage(this, msgnum);
        if(message != NULL){
            frame->setType(ft02_Message);
            frame->packInt(boardid);
            frame->packInt(msgnum);
            message->pack(frame);
        }else{
            frame->createFailFrame(fec_TempUnavailable, "Could not get Message at this time");
            Logger::getLogger()->warning("Board has messages but persistence didn't get it");
            Logger::getLogger()->warning("POSSIBLE DATABASE INCONSISTANCE");
        }
  }else{
    frame->createFailFrame(fec_NonExistant, "No such Message on board");
  }
}

long long Board::getModTime() const{
    return modtime;
}

uint32_t Board::getNumMessages() const{
    return nummsg;
}

void Board::setNumMessages(uint32_t nnm){
    nummsg = nnm;
}

void Board::setModTime(uint64_t nmt){
    modtime = nmt;
}
