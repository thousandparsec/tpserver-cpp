/*  Messages on boards
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
#include "game.h"

#include "message.h"

Message::Message(){
  turnnum = Game::getGame()->getTurnNumber();
}

int Message::getTurn(){
  return turnnum;
}

void Message::setSubject(const std::string & nsub){
  subject = nsub;
}

std::string Message::getSubject(){
  return subject;
}

void Message::setBody(const std::string & nbody){
  body = nbody;
}

std::string Message::getBody(){
  return body;
}

void Message::addReference(int type, unsigned int value){
  references.insert(std::pair<int, unsigned int>(type, value));
}

void Message::pack(Frame * frame){
  frame->packInt(1);
  frame->packInt(0);
  frame->packString(subject.c_str());
  frame->packString(body.c_str());
  frame->packInt(turnnum);
  frame->packInt(references.size());
  for(std::set<std::pair<int, unsigned int> >::iterator itcurr = references.begin(); itcurr != references.end(); ++itcurr){
    frame->packInt((*itcurr).first);
    frame->packInt((*itcurr).second);
  }
}
