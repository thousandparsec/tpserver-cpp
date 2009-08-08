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

Message::Message() : ProtocolObject( ft02_Message, 0 )
{
  turn_number = Game::getGame()->getTurnNumber();
}

int Message::getTurn() {
  return turn_number;
}

void Message::setTurn(uint32_t nt) {
  turn_number = nt;
}

void Message::setSubject(const std::string & nsub) {
  setName( nsub );
}

std::string Message::getSubject() {
  return getName();
}

void Message::setBody(const std::string & nbody) {
  setDescription( nbody );
}

std::string Message::getBody() {
  return getDescription();
}

void Message::addReference(RefSysType type, uint32_t value) {
  references.insert( Ref(type, value) );
}

Message::References Message::getReferences() const {
  return references;
}

void Message::pack(Frame * frame) const {
  if (frame->getVersion() == fv0_2) {
    frame->packInt(1);
    frame->packInt(0);
  } else {
    frame->packInt(0);
  }
  frame->packString(name);
  frame->packString(desc);
  if (frame->getVersion() > fv0_2) {
    frame->packInt(turn_number);
    frame->packInt(references.size());
    for (References::iterator itcurr = references.begin(); itcurr != references.end(); ++itcurr) {
      frame->packInt((*itcurr).first);
      frame->packInt((*itcurr).second);
    }
  }
}

