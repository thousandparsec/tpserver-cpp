/*  BoardManager
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include "boardmanager.h"

BoardManager::BoardManager(){
    nextbid = 1;
    nextmid = 1;
}

BoardManager::~BoardManager(){

}

void BoardManager::init(){
    Persistence* persist = Game::getGame()->getPersistence();
    nextbid = persist->getMaxBoardId() + 1;
    nextmid = persist->getMaxMessageId() + 1;
    std::set<uint32_t> bidset(persist->getBoardIds());
    for(std::set<uint32_t>::iterator itcurr = bidset.begin(); itcurr != bidset.end(); ++itcurr){
        boards[*itcurr] = NULL;
        boardmessages[*itcurr] = std::list<uint32_t>();
    }
}

Board* BoardManager::createNewBoard(const std::string &name, const std::string &desc){
    Board *rtn = NULL;
    rtn = new Board();
    rtn->setBoardID(nextbid++);
    rtn->setName(name);
    rtn->setDescription(desc);

    boards[rtn->getBoardID()] = (rtn);
    boardmessages[rtn->getBoardID()] = std::list<uint32_t>();
    Game::getGame()->getPersistence()->saveBoard(rtn);

    return rtn;
}

Board* BoardManager::getBoard(uint32_t id){
    Board* rtn = NULL;
    std::map<unsigned int, Board*>::iterator pl = boards.find(id);
    if(pl != boards.end()){
        rtn = (*pl).second;
    }
    if(rtn == NULL){
        rtn = Game::getGame()->getPersistence()->retrieveBoard(id);
        boards[id] = rtn;
    }
    return rtn;
}

void BoardManager::updateBoard(uint32_t id){
    Game::getGame()->getPersistence()->updateBoard(boards[id]);
}

std::set<uint32_t> BoardManager::getAllBoardIds(){
    std::set<unsigned int> vis;
    for(std::map<unsigned int, Board*>::const_iterator itid = boards.begin();
        itid != boards.end(); ++itid){
        vis.insert(itid->first);
    }
    return vis;
}

bool BoardManager::addMessage(Message* msg, Board* board, uint32_t pos){
    uint32_t msgid = nextmid++;
    messagecache[msgid] = msg;
    std::list<uint32_t> bmlist = boardmessages[board->getBoardID()];
        if(bmlist.empty()){
            bmlist = Game::getGame()->getPersistence()->retrieveOrderList(board->getBoardID());
            boardmessages[board->getBoardID()] = bmlist;
        }
        if (pos == 0xffffffff) {
            bmlist.push_back(msgid);
        } else {
            std::list<uint32_t>::iterator inspos = bmlist.begin();
            advance(inspos, pos);
            bmlist.insert(inspos, msgid);
        }
        boardmessages[board->getBoardID()] = bmlist;
        Game::getGame()->getPersistence()->saveMessage(msgid, msg);
        Game::getGame()->getPersistence()->saveMessageList(board->getBoardID(), bmlist);
        board->setNumMessages(bmlist.size());
        return true;
}

bool BoardManager::removeMessage(Board* board, uint32_t pos){
    std::list<uint32_t> bmlist = boardmessages[board->getBoardID()];
    if(bmlist.empty()){
        bmlist = Game::getGame()->getPersistence()->retrieveMessageList(board->getBoardID());
        boardmessages[board->getBoardID()] = bmlist;
    }
    if (pos >= bmlist.size()) {
        return false;
    }

    std::list<uint32_t>::iterator itpos = bmlist.begin();
    advance(itpos, pos);
    uint32_t msgid = *itpos;
    Message* msg = messagecache[msgid];
    if(msg == NULL){
        msg = Game::getGame()->getPersistence()->retrieveMessage(msgid);
        messagecache[msgid] = msg;
    }
    delete msg;
    bmlist.erase(itpos);
    boardmessages[board->getBoardID()] = bmlist;
    Game::getGame()->getPersistence()->removeMessage(msgid);
    Game::getGame()->getPersistence()->saveMessageList(board->getBoardID(), bmlist);
    board->setNumMessages(bmlist.size());
    return true;
}

Message* BoardManager::getMessage(Board* board, uint32_t pos){
    std::list<uint32_t> bmlist = boardmessages[board->getBoardID()];
    if(bmlist.empty()){
        bmlist = Game::getGame()->getPersistence()->retrieveMessageList(board->getBoardID());
        boardmessages[board->getBoardID()] = bmlist;
    }
    if (pos >= bmlist.size()) {
        return NULL;
    }

    std::list<uint32_t>::iterator itpos = bmlist.begin();
    advance(itpos, pos);
    uint32_t msgerid = *itpos;
    Message* msg = messagecache[msgerid];
    if(msg == NULL){
        msg = Game::getGame()->getPersistence()->retrieveMessage(msgerid);
        messagecache[msgerid] = msg;
    }
    return msg;
}
