/*  Player Http Connection object, supports ipv4 and ipv6
 *
 *  Copyright (C) 2006, 2007  Lee Begg and the Thousand Parsec Project
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

#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef VERSION
#define VERSION "0.0.0"
#endif


#include "logging.h"
#include "net.h"
#include "frame.h"
#include "game.h"
#include "player.h"

#include "playerhttpconn.h"
#include "systemexception.h"


PlayerHttpConnection::PlayerHttpConnection(int fd) : PlayerConnection(fd), httpbuff()
{

}

PlayerHttpConnection::~PlayerHttpConnection()
{
	if (status != DISCONNECTED) {
		close();
	}
}


int32_t PlayerHttpConnection::verCheckLastChance()
{
  int32_t rtn = -1;
  if(memcmp(rheaderbuff, "POST", 4) == 0 || memcmp(rheaderbuff, "GET ", 4) == 0){
    bool found = httpbuff.find("\r\n\r\n") != httpbuff.npos;
    if(!found){
      char* buff = new char[1024];
      try {
        int32_t len = underlyingRead(buff, 1024);
        if(len == 0){
          Logger::getLogger()->info("Client disconnected");
          close();
          rtn = 0;
        }else if(len > 0){
          httpbuff.append(buff, len);
          found = httpbuff.find("\r\n\r\n") != httpbuff.npos;
        }else{
          rtn = -2;
        }
      } catch ( SystemException& e ) {
        WARNING("Socket error -- %s", e.what());
        close();
        rtn = 0;
      }
      delete[] buff;
    }
    
    if(found){
      //Have the end of the headers
      int offset = rheaderbuff[0] == 'P' ? 1 : 0;
      std::string url = httpbuff.substr(offset, httpbuff.find(' ', offset));
      Logger::getLogger()->debug("Http url: %s", url.c_str());
      std::string response = "HTTP/1.0 200 OK\r\n";
      response += "Cache-Control: no-store, no-cache, must-revalidate, post-check=0, pre-check=0\r\n";
      response += "Pragma: no-cache\r\n\r\n";
      if(rheaderbuff[0] == 'G' || url == "/"){
        response += "<html><head><title>tpserver-cpp</title></head><body><p>Nothing to see here, move along</p></body></html>\n";
        sendDataAndClose(response.c_str(), response.length());
        rtn = 1;
      }else{
        sendData(response.c_str(), response.length());
        rtn = 1;
        httpbuff.clear();
      }
    }
  }
  return rtn;
}


