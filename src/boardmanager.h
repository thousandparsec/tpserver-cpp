#ifndef BOARDMANAGER_H
#define BOARDMANAGER_H
/* BoardManager class
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

#include <map>
#include <string>

class Board;
class Message;

class BoardManager{
public:
    BoardManager();
    ~BoardManager();

    void init();

    Board* createNewBoard(const std::string &name, const std::string &desc);
    Board* getBoard(uint32_t id);
    void updateBoard(uint32_t id);

    std::set<uint32_t> getAllBoardIds();

    // these methods *only* used by Board
    bool addMessage(Message* msg, Board* board, uint32_t pos);
    bool removeMessage(Board* board, uint32_t pos);
    Message* getMessage(Board* board, uint32_t pos);

private:
    std::map<uint32_t, Board*> boards;
    uint32_t nextbid;
    uint32_t nextmid;
    std::map<uint32_t, Message*> messagecache;
    std::map<uint32_t, std::list<uint32_t> > boardmessages;
};

#endif
