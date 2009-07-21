/*  Listen socket for tpserver-cpp with ipv4 and ipv6 support
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

#include <stdint.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "playerconnection.h"
#include "net.h"

#include "listensocket.h"

ListenSocket::ListenSocket(Type aType) : Connection(aType){
}

ListenSocket::~ListenSocket(){
  if(status != DISCONNECTED)
    close(sockfd);
  status = DISCONNECTED;
}

void ListenSocket::openListen(const std::string &address, const std::string &port){
 
  status = PRECONNECTED;

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

	int n = getaddrinfo(c_addr, port.c_str(), &hints, &res);
	
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
            fcntl(sockfd, F_SETFL, O_NONBLOCK);
            int yes = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
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
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
        int yes = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
#endif
	if (sockfd == -1){
		Logger::getLogger()->warning("Could not create Socket");
            status = DISCONNECTED;
            return;
        }
        
        //record the port for history
        portnum = atoi(port.c_str());
        
#ifndef HAVE_IPV6
	struct sockaddr_in myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(portnum);
        if(c_addr != NULL){
            struct hostent *he = gethostbyname(c_addr);
            if(he == NULL){
                Logger::getLogger()->warning("Could not lookup host name to listen on, listening on all");
                myAddr.sin_addr.s_addr = INADDR_ANY;
            }else{
                myAddr.sin_addr = *((struct in_addr *)he->h_addr);
            }
        }else{
            myAddr.sin_addr.s_addr = INADDR_ANY;
        }
	memset(&(myAddr.sin_zero), '\0', 8);

	if (bind(sockfd, (struct sockaddr *) &myAddr, sizeof(struct sockaddr)) != 0) {
		perror("bind");
		Logger::getLogger()->error("Failed to bind to port and address");
                status = DISCONNECTED;
                close(sockfd);
                return;
	}
#endif

	if (listen(sockfd, 5) != 0){
		Logger::getLogger()->error("Failed to listen");
            status = DISCONNECTED;
            close(sockfd);
            return;
        }

	if(sockfd == -1){
	  status = DISCONNECTED;
	}
    
}

void ListenSocket::process(){
    Logger::getLogger()->info("Accepting new a connection");
    try{
      Connection::Ptr temp;
#ifdef HAVE_IPV6
        struct sockaddr_storage clientaddr;
        socklen_t addrlen = sizeof(clientaddr);

        int thenewfd = accept(sockfd, 
                                                        (struct sockaddr *)&clientaddr, 
                                                        &addrlen);
        if(thenewfd != -1){
            char* clienthost = new char[NI_MAXHOST];
            char* clientname = new char[NI_MAXHOST];
            getnameinfo((struct sockaddr *)&clientaddr, addrlen,
                        clienthost, NI_MAXHOST,
                        NULL, 0,
                        NI_NUMERICHOST);
            getnameinfo((struct sockaddr *)&clientaddr, addrlen,
                        clientname, NI_MAXHOST,
                        NULL, 0,
                        0);
            
            Logger::getLogger()->info("Accepted connection from %s [%s], connection id %d", 
                                        clientname, clienthost, thenewfd);
            delete[] clienthost;
            delete[] clientname;
            temp = acceptConnection(thenewfd);
        }else{
            Logger::getLogger()->info("Not accepting new connection, accept error.");
        }
#else
        
        temp = acceptConnection(accept(sockfd, NULL, 0));
#endif
        if(temp)
            Network::getNetwork()->addConnection(temp);
    }catch(std::exception e){
        Logger::getLogger()->warning("Could not establish connection");
    }

}

uint16_t ListenSocket::getPort() const{
  return portnum;
}

