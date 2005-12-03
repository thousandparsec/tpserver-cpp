/*  Network abstraction for tpserver-cpp 
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
#include "tcpsocket.h"
#include "game.h"
#include "frame.h"

#ifdef HAVE_LIBGNUTLS
#include "tlssocket.h"
#include "playertlsconn.h"
#endif

#include "net.h"


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

void Network::addConnection(Connection* conn)
{
  Logger::getLogger()->debug("Adding a file descriptor %d", conn->getFD());
  connections[conn->getFD()] = conn;
  FD_SET(conn->getFD(), &master_set);
  if (max_fd < conn->getFD()) {
    max_fd = conn->getFD();
  }
}

void Network::removeConnection(Connection* conn)
{
  Logger::getLogger()->debug("Removing a file descriptor %d", conn->getFD());
  FD_CLR(conn->getFD(), &master_set);
  connections.erase(connections.find(conn->getFD()));
  if (max_fd == conn->getFD()) {
    Logger::getLogger()->debug("Changing max_fd");
    max_fd = 0;
    std::map < int, Connection * >::iterator itcurr, itend;
    itend = connections.end();
    for (itcurr = connections.begin(); itcurr != itend; itcurr++) {
      
      if (max_fd < (*itcurr).first)
	max_fd = (*itcurr).first;
      
    }
    
  }
}

void Network::start()
{
	if (active == true) {
		Logger::getLogger()->warning("Network already running");
		return;
	}

	if(Game::getGame()->isLoaded()){
	  Logger::getLogger()->info("Starting Network");
	  
	  TcpSocket* listensocket = new TcpSocket(Settings::getSettings()->get("listen_addr"), Settings::getSettings()->get("listen_port"));
	  if(listensocket->getStatus() != 0){
	    addConnection(listensocket);
	    active = true;
	  }else{
	    delete listensocket;
	    Logger::getLogger()->warning("Not starting network, listen socket couldn't be opened");
	  }
#ifdef HAVE_LIBGNUTLS
            TlsSocket* secsocket = new TlsSocket(Settings::getSettings()->get("secure_addr"), Settings::getSettings()->get("secure_port"));
             if(secsocket->getStatus() != 0){
                addConnection(secsocket);
                active = true;
            }else{
                delete secsocket;
                Logger::getLogger()->warning("Not starting sec network, listen socket couldn't be opened");
            }
#endif
	}else{
	   Logger::getLogger()->warning("Not starting network, game not yet loaded");
	}
}


void Network::stop()
{
	if (active) {
		Logger::getLogger()->info("Stopping Network");

		if(halt){
		  std::map<int, Connection*>::iterator itcurr = connections.begin();
		  while (itcurr != connections.end()) {
		    PlayerConnection* pc = dynamic_cast<PlayerConnection*>(itcurr->second);
		    if(pc != NULL){
		      ++itcurr;
		      pc->close();
		      removeConnection(pc);
		      delete pc;
		    }else{
		      TcpSocket* ts = dynamic_cast<TcpSocket*>(itcurr->second);
		      if(ts != NULL){
			++itcurr;
			removeConnection(ts);
			delete ts;
		      }else{
#ifdef HAVE_LIBGNUTLS
                        TlsSocket* sts = dynamic_cast<TlsSocket*>(itcurr->second);
                        if(sts != NULL){
                            ++itcurr;
                            removeConnection(sts);
                            delete sts;
                        }else{
#endif
                            ++itcurr;
#ifdef HAVE_LIBGNUTLS
                        }
#endif
		      }
		    }
		  }

		}

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
	halt = false;
	while (!halt) {

		//sleep(1);
	  bool netstat = active;

		cur_set = master_set;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		if (select(max_fd + 1, &cur_set, NULL, NULL, &tv) != 0) {

			std::map < int, Connection * >::iterator itcurr;
			for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
				if (FD_ISSET((*itcurr).first, &cur_set)) {
					(*itcurr).second->process();
				}
				if ((*itcurr).second->getStatus() == 0) {
				  Logger::getLogger()->info("Closed connection %d", (*itcurr).second->getFD());
				  Connection* conn = itcurr->second;
				  removeConnection(conn);
				}
			}

		}

		if(netstat != active && active == false){
		  std::map<int, Connection*>::iterator itcurr = connections.begin();
		  while (itcurr != connections.end()) {
		    PlayerConnection* pc = dynamic_cast<PlayerConnection*>(itcurr->second);
		    if(pc != NULL){
		      ++itcurr;
		      pc->close();
		      removeConnection(pc);
		      delete pc;
		    }else{
		      TcpSocket* ts = dynamic_cast<TcpSocket*>(itcurr->second);
		      if(ts != NULL){
			++itcurr;
			removeConnection(ts);
			delete ts;
		      }else{
			++itcurr;
		      }
		    }
		  }
		  Logger::getLogger()->debug("Network really stopped");
		}

		if(Game::getGame()->secondsToEOT() <= 0){
		  Game::getGame()->doEndOfTurn();
		}


	}




}

void Network::stopMainLoop(){
  halt = true;
}


Network::Network()
{

  max_fd = 0;

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
