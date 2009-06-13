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
#include <tpserver/board.h>

/**
 * Manager class for boards and messages
 */
class BoardManager{
public:
    /**
     * Default constructor
     */
    BoardManager();

    /**
     * Initializes the board manager
     *
     * Prepares board data.
     */
    void init();

    /** 
     * Creates a new board
     *
     * @param name Board name
     * @param desc Board description
     *
     * @returns Pointer to newly created board
     */
    Board::Ptr createNewBoard(const std::string &name, const std::string &desc);

    /**
     * Returns board by ID
     */
    Board::Ptr getBoard(uint32_t id);

    /**
     * Posts message to top of passed board, by boardid.
     */
    void postToBoard(Message* msg, uint32_t boardid);

    // these methods *only* used by Board
    uint32_t addMessage(Message* msg);
    bool removeMessage(uint32_t message_id);
    Message* getMessage(uint32_t message_id);

private:
    typedef std::map<uint32_t, Board::Ptr>   BoardMap;
    typedef std::map<uint32_t, Message*> MessageMap;
    
    BoardMap boards;
    MessageMap messagecache;
    
    uint32_t nextbid;
    uint32_t nextmid;
};

#endif
