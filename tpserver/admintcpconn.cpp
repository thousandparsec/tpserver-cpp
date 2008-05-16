/*  Admin Tcp Connection object, supports ipv4 and ipv6
 *
 *  Copyright (C) 2008 Aaron Mavrinac and the Thousand Parsec Project
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef VERSION
#define VERSION "0.0.0"
#endif


#include "logging.h"
#include "net.h"

#include "admintcpconn.h"

AdminTcpConnection::AdminTcpConnection() : AdminConnection(), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0), sbuff(NULL), sbuffused(0), sbuffsize(0), sendqueue(), sendandclose(false)
{

}


AdminTcpConnection::AdminTcpConnection(int fd) : AdminConnection(fd), rheaderbuff(NULL), rdatabuff(NULL), rbuffused(0), sbuff(NULL), sbuffused(0), sbuffsize(0), sendqueue(), sendandclose(false)
{
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

AdminTcpConnection::~AdminTcpConnection()
{
	if (status != 0) {
		close();
	}
        if(rheaderbuff != NULL)
          delete[] rheaderbuff;
        if(rdatabuff != NULL)
          delete[] rdatabuff;
        if(sbuff != NULL)
          delete[] sbuff;
        while(!sendqueue.empty()){
          delete sendqueue.front();
          sendqueue.pop();
        }
}


void AdminTcpConnection::close()
{
  if(sendqueue.empty()){
    Logger::getLogger()->debug("Closing connection");
    ::close(sockfd);
    status = 0;
  }else{
    sendandclose = true;
  }
}

void AdminTcpConnection::sendDataAndClose(const char* data, uint32_t size){
  sendData(data, size);
  close();
}

void AdminTcpConnection::sendData(const char* data, uint32_t size){
  while(!sendqueue.empty()){
    delete sendqueue.front();
    sendqueue.pop();
  }
  sbuff = new char[size];
  memcpy(sbuff, data, size);
  sbuffsize = size;
  sbuffused = 0;
  sendFrame(new Frame(fv0_3));
}

int32_t AdminTcpConnection::underlyingRead(char* buff, uint32_t size){
  int32_t len = recv(sockfd, buff, size, 0);
  if(len < 0){
    if(errno != EAGAIN && errno != EWOULDBLOCK){
      Logger::getLogger()->error("underlying read, tcp, error is: %s", strerror(errno));
      len = -1;
    }else{
      len = -2;
    }
  }
  return len;
}

int32_t AdminTcpConnection::underlyingWrite(const char* buff, uint32_t size){
  int len = send(sockfd, buff, size, 0);
  if(len < 0){
    if(errno != EAGAIN && errno != EWOULDBLOCK){
      Logger::getLogger()->error("underlying write, tcp, error is: %s", strerror(errno));
      len = -1;
    }else{
      len = -2;
    }
  }
  return len;
}
