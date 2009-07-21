/*  Tcp listen socket for tpserver-cpp with ipv4 and ipv6 support
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include <string>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "playerconnection.h"
#include "net.h"

#include "tcpsocket.h"

TcpSocket::TcpSocket() : ListenSocket(LISTEN){
}

TcpSocket::~TcpSocket(){
}

void TcpSocket::openListen(const std::string& address, const std::string& port){ 
    if(port.length() == 0){
        ListenSocket::openListen(address, "6923");
    }else{
        ListenSocket::openListen(address, port);
    }
}


Connection::Ptr TcpSocket::acceptConnection(int fd){
    Logger::getLogger()->info("Accepting new tp (tcp) connection");

    return Connection::Ptr( new PlayerConnection(fd) );
}
