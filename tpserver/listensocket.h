#ifndef LISTENSOCKET_H
#define LISTENSOCKET_H
/* Server Listen socket connection
 *
 *  Copyright (C) 2005, 2006  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/connection.h>

//class PlayerConnection;

class ListenSocket : public Connection {
  public:
    /// Shared pointer typedef
    typedef boost::shared_ptr< ListenSocket > Ptr;

    ListenSocket();
    virtual ~ListenSocket();

    virtual void openListen(const std::string& address, const std::string& port);

    virtual void process();

    uint16_t getPort() const;

    bool isPlayer();

  protected:
    virtual Connection::Ptr acceptConnection(int fd) = 0;

    bool player;

  private:
    uint16_t portnum;

};

#endif
