/*  Messages boards
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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

#include "board.h"


void Board::setBoardID(int i){
  boardid = i;
}

int Board::getBoardID(){
  return boardid;
}

void Board::setName(const std::string & nname){
  name = nname;
}

std::string Board::getName(){
  return name;
}

void Board::setDescription(const std::string & ndest){
  description = ndest;
}

std::string Board::getDescription(){
  return description;
}

void Board::addMessage(Message * msg, int pos){
  if(pos == -1){
    messages.push_back(msg);
  }else{
    std::list<Message*>::iterator itpos = messages.begin();
    advance(itpos, pos);
    messages.insert(itpos, msg);
  }
}

bool Board::removeMessage(unsigned int pos){
  if(pos >= messages.size() || pos < 0){
    return false;
  }
  std::list<Message*>::iterator itpos = messages.begin();
  advance(itpos, pos);
  delete (*itpos);
  messages.erase(itpos);
  return true;
}

void Board::packBoard(Frame * frame){
  frame->setType(ft02_Board);
  frame->packInt(boardid);
  frame->packString(name.c_str());
  frame->packString(description.c_str());
  frame->packInt(messages.size());
}

void Board::packMessage(Frame * frame, unsigned int msgnum){
  if(msgnum < messages.size()){
    frame->setType(ft02_Message);
    frame->packInt(boardid);
    frame->packInt(msgnum);
    std::list<Message*>::iterator itpos = messages.begin();
    advance(itpos, msgnum);
    (*itpos)->pack(frame);
  }else{
    frame->createFailFrame(fec_NonExistant, "No such Message on board");
  }
}
