/*  Network abstraction for tpserver-cpp 
 *
 *  Copyright (C) 2003-2005, 2006  Lee Begg and the Thousand Parsec Project
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
#include "settingscallback.h"
#include "connection.h"
#include "playerconnection.h"
#include "tcpsocket.h"
#include "game.h"
#include "frame.h"
#include "httpsocket.h"

#ifdef HAVE_LIBGNUTLS
#include "tlssocket.h"
#include "httpssocket.h"
#endif

#ifdef HAVE_AVAHI
#include "avahi.h"
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
    frame->packInt(features.size());
    for(std::map<int,int>::iterator itcurr = features.begin(); itcurr != features.end(); ++itcurr){
      frame->packInt(itcurr->first);
    }
  }else{
    Logger::getLogger()->warning("Tryed to create a Features frame for protocol version less than 3");
    frame->createFailFrame(fec_FrameError, "Unknown request for features (not in current protocol)");
  }
}

void Network::addFeature(int featid, int value){
  features[featid] = value;
}

void Network::removeFeature(int featid){
  features.erase(featid);
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
          
#ifdef HAVE_AVAHI
          //start avahi
          try{
            avahi = new Avahi();
          }catch(std::exception e){
            avahi = NULL;
          }
#endif
          
	  uint numsocks = 0;
	  TcpSocket* listensocket = new TcpSocket();
            listensocket->openListen(Settings::getSettings()->get("tp_addr"), Settings::getSettings()->get("tp_port"));
	  if(listensocket->getStatus() != 0){
	    addConnection(listensocket);
	    numsocks++;
#ifdef HAVE_AVAHI
            if(avahi != NULL)
              avahi->addService("tp", listensocket->getPort());
#endif
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
              addFeature(fid_http_other, atoi(Settings::getSettings()->get("http_port").c_str()));
#ifdef HAVE_AVAHI
              if(avahi != NULL)
                avahi->addService("tphttp", httpsocket->getPort());
#endif
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
                    addFeature(fid_sec_conn_other, atoi(Settings::getSettings()->get("tps_port").c_str()));
#ifdef HAVE_AVAHI
                    if(avahi != NULL)
                      avahi->addService("tps", secsocket->getPort());
#endif
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
#ifdef HAVE_AVAHI
                    if(avahi != NULL)
                      avahi->addService("tphttps", secsocket->getPort());
#endif
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

	}else{
	   Logger::getLogger()->warning("Not starting network, game not yet loaded");
	}

        //set if account registration is allowed.
        if(Settings::getSettings()->get("add_players") == "yes"){
          addFeature(fid_account_register, 0);
        }else{
          removeFeature(fid_account_register);
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
                  removeFeature(fid_sec_conn_other);
                  removeFeature(fid_http_other);
#ifdef HAVE_AVAHI
                  if(avahi != NULL)
                    delete avahi;
                  avahi = NULL;
#endif
		active = false;

	} else {
		Logger::getLogger()->warning("Network already stopped");
	}
}

bool Network::isStarted() const{
  return active;
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
                                    //use select again, don't check rest of list as it has changed.
                                    break;
				}
			}

		}
                
#ifdef HAVE_AVAHI
                if(avahi != NULL){
                  avahi->poll();
                }
#endif

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
    FD_ZERO(&master_set);

	halt = false;
	active = false;
  features[fid_keep_alive] = 0;
  features[fid_serverside_property] = 0;
  avahi = NULL;
  Settings::getSettings()->setCallback("add_players", SettingsCallback(this, &Network::addAccountSettingChanged));
}


Network::~Network()
{
  if(avahi != NULL)
    delete avahi;
  
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

void Network::addAccountSettingChanged(const std::string &item, const std::string &value){
  Logger::getLogger()->debug("In addAccountSettingChanged, working");
  if(value == "yes"){
    addFeature(fid_account_register, 0);
  }else{
    removeFeature(fid_account_register);
  }
}
