/*  Network and listen socket abstraction for tpserver-cpp with ipv4 and
 *   ipv6 support
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
#include <cassert>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "settings.h"
#include "connection.h"
#include "playertcpconn.h"
#include "game.h"
#include "frame.h"

#include "net.h"

// thread start function
static void *startMaster(void *arg)
{
	Network::getNetwork()->masterLoop();
	return NULL;
}


// Network Class methods

Network *Network::myInstance = NULL;


Network *Network::getNetwork()
{
	if (myInstance == NULL) {
		myInstance = new Network();
	}
	return myInstance;
}

void Network::createFeaturesFrame(Frame* frame){
  if(frame->getVersion() >= fv0_3){
    frame->setType(ft03_Features);
    frame->packInt(1); //one optional features at this time.
    frame->packInt(5); // Keep Alive (ping frames)
                       //  TODO make consts or enums
  }else{
    Logger::getLogger()->warning("Tryed to create a Features frame for protocol version less than 3");
    frame->createFailFrame(fec_FrameError, "Unknown request for features (not in current protocol)");
  }
}

void Network::addFD(int fd)
{
  Logger::getLogger()->debug("Adding a file descriptor %d", fd);
	FD_SET(fd, &master_set);
	if (max_fd < fd) {
		max_fd = fd;
	}
}

void Network::removeFD(int fd)
{
  Logger::getLogger()->debug("Removing a file descriptor %d", fd);
	FD_CLR(fd, &master_set);
	if (max_fd == fd) {
		Logger::getLogger()->debug("Changing max_fd");
		max_fd = serverFD;
		std::map < int, Connection * >::iterator itcurr, itend;
		itend = connections.end();
		for (itcurr = connections.begin(); itcurr != itend; itcurr++) {
			if (FD_ISSET((*itcurr).first, &master_set)) {
				if (max_fd < (*itcurr).first)
					max_fd = (*itcurr).first;
			}
		}

	}
}

void Network::start()
{
	if (active == true) {
		Logger::getLogger()->warning("Network already running");
		return;
	}
	Logger::getLogger()->info("Starting Network");

#ifdef HAVE_IPV6

	struct addrinfo hints, *res, *ressave;

	memset(&hints, 0, sizeof(struct addrinfo));
	
	hints.ai_flags    = AI_PASSIVE;
	hints.ai_family   = PF_UNSPEC; 
	hints.ai_socktype = SOCK_STREAM;

	int n = getaddrinfo(NULL, "6923", &hints, &res); // should be conf item
	
	if (n < 0) {
	  fprintf(stderr, "getaddrinfo error:: [%s]\n", gai_strerror(n));
	  Logger::getLogger()->error("Could not getaddrinfo");
	}
	
	ressave=res;

	serverFD=-1;
	while (res) {
	  serverFD = socket(res->ai_family,
			  res->ai_socktype,
			  res->ai_protocol);

	  if (!(serverFD < 0)) {
            if (bind(serverFD, res->ai_addr, res->ai_addrlen) == 0)
	      break;
	    
            close(serverFD);
            serverFD=-1;
	  }
	  res = res->ai_next;
	}

	freeaddrinfo(ressave);
#else
	serverFD = socket(AF_INET, SOCK_STREAM, 0);
#endif
	if (serverFD == -1)
		Logger::getLogger()->error("Could not create Socket");
#ifndef HAVE_IPV6
	struct sockaddr_in myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(6923);	// fix, make setting item
	myAddr.sin_addr.s_addr = INADDR_ANY;	// fix, make setting item
	memset(&(myAddr.sin_zero), '\0', 8);

	if (bind(serverFD, (struct sockaddr *) &myAddr, sizeof(struct sockaddr)) != 0) {
		perror("bind");
		Logger::getLogger()->error("Failed to bind to port and address");
	}
#endif

	if (listen(serverFD, 5) != 0)
		Logger::getLogger()->error("Failed to listen");

	FD_ZERO(&master_set);
	FD_SET(serverFD, &master_set);
	max_fd = serverFD;

	// time to create thread
	halt = false;
	pthread_create(&master, NULL, startMaster, NULL);



	active = true;
}


void Network::stop()
{
	if (active) {
		Logger::getLogger()->info("Stopping Network");
		close(serverFD);

		//more to come
		halt = true;
		pthread_join(master, NULL);

		active = false;

	} else {
		Logger::getLogger()->warning("Network already stopped");
	}
}

void Network::sendToAll(Frame * frame){
  std::map < int, Connection * >::iterator itcurr;
  char* data = frame->getData();
  for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
    PlayerConnection * currConn = dynamic_cast<PlayerConnection*>(itcurr->second);
    if(currConn != NULL && currConn->getStatus() == 3){
      Frame * currFrame = currConn->createFrame(NULL);
      currFrame->setType(frame->getType());
      currFrame->setData(data, frame->getDataLength());
      
      currConn->sendFrame(currFrame);

    }
  }
  free(data);
  delete frame;
}

void Network::masterLoop()
{
	struct timeval tv;
	fd_set cur_set;
	while (!halt) {

		//sleep(1);

		cur_set = master_set;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		if (select(max_fd + 1, &cur_set, NULL, NULL, &tv) != 0) {

			if (FD_ISSET(serverFD, &cur_set)) {
				Logger::getLogger()->info("Accepting new connection");

#ifdef HAVE_IPV6
				struct sockaddr_storage clientaddr;
				socklen_t addrlen = sizeof(clientaddr);
				Connection *temp = new PlayerTcpConnection(accept(serverFD, 
									 (struct sockaddr *)&clientaddr, 
									 &addrlen));
				connections[temp->getFD()] = temp;
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
				connections[temp->getFD()] = temp;

#endif
			}

			std::map < int, Connection * >::iterator itcurr;
			for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
				if (FD_ISSET((*itcurr).first, &cur_set)) {
					(*itcurr).second->process();
				}
				if ((*itcurr).second->getStatus() == 0) {
				  Logger::getLogger()->info("Closed connection %d", (*itcurr).second->getFD());
					delete(*itcurr).second;
					std::map < int, Connection * >::iterator temp;
					temp = itcurr;
					--temp;
					connections.erase(itcurr);
					itcurr = temp;
				}
			}

		}
		if(Game::getGame()->secondsToEOT() <= 0){
		  Game::getGame()->doEndOfTurn();
		}


	}

	while (!connections.empty()) {
	  PlayerConnection* pc = dynamic_cast<PlayerConnection*>((connections.begin())->second);
	  if(pc != NULL){
	    pc->close();
	  }
	  delete(*connections.begin()).second;
	  connections.erase(connections.begin());
	}


}




Network::Network()
{



	halt = false;
	active = false;
}


Network::~Network()
{




}


Network::Network(Network & rhs)
{
}


Network Network::operator=(Network & rhs)
{
  // please don't call me
  assert(0);
  return *this;
}
