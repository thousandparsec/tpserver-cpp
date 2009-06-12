/*  BoardManager
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

#include "board.h"
#include "game.h"
#include "persistence.h"
#include "message.h"
#include "logging.h"
#include "frame.h"

#include "boardmanager.h"

BoardManager::BoardManager() 
  : nextbid(1), nextmid(1) {
}


void BoardManager::init() {
  Persistence* persist = Game::getGame()->getPersistence();
  nextbid = persist->getMaxBoardId() + 1;
  nextmid = persist->getMaxMessageId() + 1;
  std::set<uint32_t> bidset(persist->getBoardIds());

  for(std::set<uint32_t>::iterator itcurr = bidset.begin(); itcurr != bidset.end(); ++itcurr) {
    boards[*itcurr] = NULL;
  }
}

Board* BoardManager::createNewBoard(const std::string &name, const std::string &desc) {
  Board *rtn = new Board(nextbid++, name, desc);

  boards[rtn->getBoardID()] = (rtn);
  Game::getGame()->getPersistence()->saveBoard(rtn);

  return rtn;
}

Board* BoardManager::getBoard(uint32_t id) {
  Board* rtn = NULL;
  BoardMap::iterator pl = boards.find(id);
  if (pl != boards.end()) {
    rtn = (*pl).second;
  }
  if (rtn == NULL) {
    rtn = Game::getGame()->getPersistence()->retrieveBoard(id);
    boards[id] = rtn;
  }
  return rtn;
}

void BoardManager::postToBoard(Message* msg, uint32_t boardid) {
  Board* board = getBoard(boardid);
  board->addMessage(msg,-1);
}

uint32_t BoardManager::addMessage(Message* msg) {
  uint32_t msgid = nextmid++;
  messagecache[msgid] = msg;
  if (!(Game::getGame()->getPersistence()->saveMessage(msgid, msg))) {
    messagecache[msgid] = NULL;
    // signal that the message is invalid
    return UINT32_NEG_ONE;
  }
  return msgid;
}


bool BoardManager::removeMessage(uint32_t message_id) {
  if (!(Game::getGame()->getPersistence()->removeMessage(message_id))) {
    // better to keep the message than get a segfault...
    return false;
  }
  if ( messagecache[message_id] != NULL ) {
    delete messagecache[message_id];
    messagecache[message_id] = NULL;
  }
  return true;
}

Message* BoardManager::getMessage(uint32_t message_id) {
  if (messagecache[message_id] == NULL) {
    messagecache[message_id] = Game::getGame()->getPersistence()->retrieveMessage(message_id);
  }
  return messagecache[message_id];
}

