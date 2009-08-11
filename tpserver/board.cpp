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
#include "frameexception.h"
#include "message.h"
#include "game.h"
#include "boardmanager.h"
#include "persistence.h"
#include "logging.h"

#include "board.h"

Board::Board(uint32_t nid, const std::string& nname, const std::string& ndesc) 
  : ProtocolObject( ft02_Board, nid, nname, ndesc )
{
  message_count = 0;
}

void Board::addMessage(Message::Ptr msg, int pos) {
  retrieveMessageList();
  uint32_t msgid = Game::getGame()->getBoardManager()->addMessage(msg);
  if (msgid == UINT32_NEG_ONE)
  {
    // error report?
    return;
  }
  if (pos == -1) {
    message_ids.push_back(msgid);
  } else {
    IdList::iterator inspos = message_ids.begin();
    advance(inspos, pos);
    message_ids.insert(inspos, msgid);
  }
  message_count = message_ids.size();
  Game::getGame()->getPersistence()->saveMessageList(id, message_ids);
  Game::getGame()->getPersistence()->updateBoard(shared_from_this());
  touchModTime();
}

bool Board::removeMessage(uint32_t pos){
  retrieveMessageList();
  if (pos >= message_ids.size()) {
    return false;
  }
  IdList::iterator itpos = message_ids.begin();
  advance(itpos, pos);
  uint32_t msgid = *itpos;
  message_ids.erase(itpos);
  message_count = message_ids.size();

  bool result = true;
  result = result && Game::getGame()->getBoardManager()->removeMessage(msgid);
  result = result && Game::getGame()->getPersistence()->saveMessageList(id, message_ids);
  Game::getGame()->getPersistence()->updateBoard(shared_from_this());

  if (result) {
    touchModTime();
  }
  return true;
}

void Board::pack(OutputFrame * frame) const {
  ProtocolObject::pack( frame );
  frame->packInt(message_count);
  frame->packInt64(getModTime());
}

void Board::packMessage(OutputFrame * frame, uint32_t msgnum) {
  if (msgnum < message_count) {
    Message::Ptr message;
    retrieveMessageList();
    if (msgnum < message_ids.size()) {
      IdList::iterator itpos = message_ids.begin();
      advance(itpos, msgnum);
      message = Game::getGame()->getBoardManager()->getMessage(*itpos);
    }
    if (message != NULL) {
      frame->setType(ft02_Message);
      frame->packInt(id);
      frame->packInt(msgnum);
      message->pack(frame);
    } else {
      WARNING("Board has messages but persistence didn't get it");
      WARNING("POSSIBLE DATABASE INCONSISTANCE");
      throw FrameException(fec_TempUnavailable, "Could not get Message at this time");
    }
  } else {
    throw FrameException(fec_NonExistant, "No such Message on board");
  }
}

uint32_t Board::getNumMessages() const{
  return message_count;
}

void Board::setPersistenceData( uint32_t nmessage_count, uint64_t nmod_time ) {
  message_count = nmessage_count;
  setModTime(nmod_time);
}

void Board::retrieveMessageList() {
  if (message_ids.empty()) {
    message_ids = Game::getGame()->getPersistence()->retrieveMessageList(id);
  }
}
