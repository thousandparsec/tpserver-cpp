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
  IdSet bidset(persist->getBoardIds());

  for(IdSet::iterator itcurr = bidset.begin(); itcurr != bidset.end(); ++itcurr) {
    boards[*itcurr] = Board::Ptr();
  }
}

Board::Ptr BoardManager::createNewBoard(const std::string &name, const std::string &desc) {
  Board::Ptr board( new Board(nextbid++, name, desc) );

  boards[board->getId()] = board;
  Game::getGame()->getPersistence()->saveBoard(board);

  return board;
}

Board::Ptr BoardManager::getBoard(uint32_t id) {
  Board::Ptr board;
  BoardMap::iterator pl = boards.find(id);
  if (pl != boards.end()) {
    board = (*pl).second;
  }
  if (board.get() == NULL) {
    board = Game::getGame()->getPersistence()->retrieveBoard(id);
    boards[id] = board;
  }
  return board;
}

void BoardManager::postToBoard(Message::Ptr msg, uint32_t boardid) {
  getBoard(boardid)->addMessage(msg,-1);
}

uint32_t BoardManager::addMessage(Message::Ptr msg) {
  uint32_t msgid = nextmid++;
  messagecache[msgid] = msg;
  if (!(Game::getGame()->getPersistence()->saveMessage(msgid, msg))) {
    messagecache[msgid].reset();
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
  messagecache[message_id].reset();
  return true;
}

Message::Ptr BoardManager::getMessage(uint32_t message_id) {
  if (messagecache[message_id] == NULL) {
    messagecache[message_id] = Game::getGame()->getPersistence()->retrieveMessage(message_id);
  }
  return messagecache[message_id];
}

