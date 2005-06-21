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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "playertcpconn.h"
#include "net.h"

#include "tcpsocket.h"

TcpSocket::TcpSocket(std::string address, std::string port){
 
  status = 1;
  
  if(port.length() == 0){
    port = "6923";
  }

  const char* c_addr = address.c_str();
  if(address.length() == 0){
    c_addr = NULL;
  }

#ifdef HAVE_IPV6

	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(struct addrinfo));
	
	hints.ai_flags    = AI_PASSIVE;
	hints.ai_family   = PF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;

	int n = getaddrinfo(c_addr, port.c_str(), &hints, &res); // should be conf item
	
	if (n < 0) {
	  fprintf(stderr, "getaddrinfo error:: [%s]\n", gai_strerror(n));
	  Logger::getLogger()->error("Could not getaddrinfo");
	}
	
	ressave=res;

	sockfd=-1;
	while (res) {
	  sockfd = socket(res->ai_family,
			  res->ai_socktype,
			  res->ai_protocol);

	  if (!(sockfd < 0)) {
            if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0)
	      break;
	    
            close(sockfd);
            sockfd=-1;
	  }
	  res = res->ai_next;
	}

	freeaddrinfo(ressave);
#else
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif
	if (sockfd == -1)
		Logger::getLogger()->warning("Could not create Socket");
#ifndef HAVE_IPV6
	struct sockaddr_in myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(6923);	// fix, take parameter
	myAddr.sin_addr.s_addr = INADDR_ANY;	// fix, take parameter
	memset(&(myAddr.sin_zero), '\0', 8);

	if (bind(sockfd, (struct sockaddr *) &myAddr, sizeof(struct sockaddr)) != 0) {
		perror("bind");
		Logger::getLogger()->error("Failed to bind to port and address");
	}
#endif

	if (listen(sockfd, 5) != 0)
		Logger::getLogger()->error("Failed to listen");

	if(sockfd == -1){
	  status = 0;
	}
}

TcpSocket::~TcpSocket(){
  if(status != 0)
    close(sockfd);
  status = 0;
}

void TcpSocket::process(){
  Logger::getLogger()->info("Accepting new connection");
  
#ifdef HAVE_IPV6
  struct sockaddr_storage clientaddr;
  socklen_t addrlen = sizeof(clientaddr);
  Connection *temp = new PlayerTcpConnection(accept(sockfd, 
						    (struct sockaddr *)&clientaddr, 
						    &addrlen));
  Network::getNetwork()->addConnection(temp);
  char clienthost[NI_MAXHOST];
  char clientname[NI_MAXHOST];
  getnameinfo((struct sockaddr *)&clientaddr, addrlen,
	      clienthost, sizeof(clienthost),
	      NULL, 0,
	      NI_NUMERICHOST);
  getnameinfo((struct sockaddr *)&clientaddr, addrlen,
	      clientname, sizeof(clientname),
	      NULL, 0,
	      0);
  
  Logger::getLogger()->info("Connection accepted from %s [%s], connection id %d", 
			    clientname, clienthost, temp->getFD());
  
#else
  
  Connection *temp = new PlayerTcpConnection(accept(serverFD, NULL, 0));
  Network::getNetwork()->addConnection(temp);
#endif
}
