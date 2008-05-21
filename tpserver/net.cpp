/*  Network abstraction for tpserver-cpp 
 *
 *  Copyright (C) 2003-2005, 2006, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <sys/select.h>
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
#include "settingscallback.h"
#include "connection.h"
#include "playerconnection.h"
#include "tcpsocket.h"
#include "game.h"
#include "frame.h"
#include "httpsocket.h"
#include "advertiser.h"
#include "timercallback.h"
#include "asyncframe.h"

#ifdef HAVE_LIBGNUTLS
#include "tlssocket.h"
#include "httpssocket.h"
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

void Network::addToWriteQueue(Connection* conn){
  writequeue[conn->getFD()] = conn;
}

void Network::addTimer(TimerCallback callback){
  timers.push(callback);
}

void Network::start()
{
	if (active == true) {
		Logger::getLogger()->warning("Network already running");
		return;
	}

	if(Game::getGame()->isLoaded()){
	  Logger::getLogger()->info("Starting Network");
          

	  uint32_t numsocks = 0;
	  TcpSocket* listensocket = new TcpSocket();
            listensocket->openListen(Settings::getSettings()->get("tp_addr"), Settings::getSettings()->get("tp_port"));
	  if(listensocket->getStatus() != 0){
	    addConnection(listensocket);
	    numsocks++;
            advertiser->addService("tp", listensocket->getPort());

	  }else{
	    delete listensocket;
	    Logger::getLogger()->warning("Could not listen on TP (tcp) socket");
	  }
          if(Settings::getSettings()->get("http") == "yes"){
            HttpSocket* httpsocket = new HttpSocket();
            httpsocket->openListen(Settings::getSettings()->get("http_addr"), Settings::getSettings()->get("http_port"));
            if(httpsocket->getStatus() != 0){
              addConnection(httpsocket);
              numsocks++;
              advertiser->addService("tp+http", httpsocket->getPort());
            }else{
              delete httpsocket;
              Logger::getLogger()->warning("Could not listen on HTTP (http tunneling) socket");
            }
          }else{
            Logger::getLogger()->info("Not configured to start http socket");
          }
#ifdef HAVE_LIBGNUTLS
            if(Settings::getSettings()->get("tps") == "yes"){
                TlsSocket* secsocket = new TlsSocket();
                secsocket->openListen(Settings::getSettings()->get("tps_addr"), Settings::getSettings()->get("tps_port"));
                if(secsocket->getStatus() != 0){
                    addConnection(secsocket);
                    numsocks++;
                    advertiser->addService("tps", secsocket->getPort());
                }else{
                    delete secsocket;
                    Logger::getLogger()->warning("Could not listen on TPS (tls) socket");
                }
            }else{
                Logger::getLogger()->info("Not configured to start tps socket");
            }
            if(Settings::getSettings()->get("https") == "yes"){
                HttpsSocket* secsocket = new HttpsSocket();
                secsocket->openListen(Settings::getSettings()->get("https_addr"), Settings::getSettings()->get("https_port"));
                if(secsocket->getStatus() != 0){
                    addConnection(secsocket);
                    numsocks++;
                    advertiser->addService("tp+https", secsocket->getPort());
                }else{
                    delete secsocket;
                    Logger::getLogger()->warning("Could not listen on HTTPS (https tunneling) socket");
                }
            }else{
                Logger::getLogger()->info("Not configured to start https socket");
            }
#endif
            if(numsocks != 0){
                Logger::getLogger()->info("Started network with %d listeners", numsocks);
                active = true;
            }

            advertiser->publish();

	}else{
	   Logger::getLogger()->warning("Not starting network, game not yet loaded");
	}



}


void Network::stop()
{
	if (active) {
		Logger::getLogger()->info("Stopping Network");

		  std::map<int, Connection*>::iterator itcurr = connections.begin();
		  while (itcurr != connections.end()) {
		    PlayerConnection* pc = dynamic_cast<PlayerConnection*>(itcurr->second);
		    if(pc != NULL){
		      ++itcurr;
		      pc->close();
		      removeConnection(pc);
		      delete pc;
		    }else{
                        ListenSocket* ts = dynamic_cast<ListenSocket*>(itcurr->second);
		      if(ts != NULL){
			++itcurr;
			removeConnection(ts);
			delete ts;
		      }else{
                        ++itcurr;
		      }
		    }
		  }

                  advertiser->unpublish();

		active = false;

	} else {
		Logger::getLogger()->warning("Network already stopped");
	}
}

bool Network::isStarted() const{
  return active;
}

void Network::sendToAll(AsyncFrame* aframe){
  std::map < int, Connection * >::iterator itcurr;
  for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
    PlayerConnection * currConn = dynamic_cast<PlayerConnection*>(itcurr->second);
    if(currConn != NULL && currConn->getStatus() == 3){
      Frame * currFrame = currConn->createFrame(NULL);
      if(aframe->createFrame(currFrame)){
        currConn->sendFrame(currFrame);
      }
    }
  }
  delete aframe;
}

void Network::doneEOT(){
  advertiser->updatePublishers();
}

Advertiser* Network::getAdvertiser() const{
  return advertiser;
}

void Network::masterLoop()
{
	struct timeval tv;
	fd_set cur_set;
	halt = false;
	while (!halt) {

		//sleep(1);
	  bool netstat = active;

                while(!timers.empty() && (timers.top().getExpireTime() <= static_cast<uint64_t>(time(NULL)) ||
							 !(timers.top().isValid()))){
                  TimerCallback callback = timers.top();
                  timers.pop();
                  if(callback.isValid())
                    callback.call();
                }
                if(timers.empty()){
                  tv.tv_sec = 60;
                  tv.tv_usec = 0;
                }else{
                  tv.tv_sec = (timers.top().getExpireTime() - time(NULL)) - 1;
                  if(tv.tv_sec <= 0){
                    tv.tv_sec = 0;
                    tv.tv_usec = 200000;
                  }else{
                    tv.tv_usec = 0;
                  }
                }
                fd_set write_set;
                FD_ZERO(&write_set);
                for(std::map<int, Connection*>::iterator itcurr = writequeue.begin();
                    itcurr != writequeue.end(); ++itcurr){
                  FD_SET(itcurr->first, &write_set);
                }

                cur_set = master_set;

		if (select(max_fd + 1, &cur_set, &write_set, NULL, &tv) > 0) {
                  
                  for(std::map<int, Connection*>::iterator itcurr = writequeue.begin();
                      itcurr != writequeue.end(); ++itcurr){
                    if(FD_ISSET(itcurr->first, &write_set)){
                      Connection* conn = itcurr->second;
                      writequeue.erase(itcurr);
                      conn->processWrite();
                      //use select again, don't check rest of list as it has changed.
                      break;
                    }
                  }

			std::map < int, Connection * >::iterator itcurr;
			for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
				if (FD_ISSET((*itcurr).first, &cur_set)) {
					(*itcurr).second->process();
				}
				if ((*itcurr).second->getStatus() == 0) {
				  Logger::getLogger()->info("Closed connection %d", (*itcurr).second->getFD());
				  Connection* conn = itcurr->second;
				  removeConnection(conn);
                                  delete conn;
                                    //use select again, don't check rest of list as it has changed.
                                    break;
				}
			}

		}

                //advertiser->poll();

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


	}




}

void Network::stopMainLoop(){
  halt = true;
}


Network::Network()
{

  max_fd = 0;
    FD_ZERO(&master_set);

	halt = false;
	active = false;

  advertiser = new Advertiser();

}


Network::~Network()
{
  delete advertiser;
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

