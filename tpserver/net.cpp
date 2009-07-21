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
#include <stdint.h>
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
#include "adminconnection.h"
#include "tcpsocket.h"
#include "admintcpsocket.h"
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

void Network::addConnection(Connection::Ptr conn)
{
  DEBUG("Adding a file descriptor %d", conn->getFD());
  connections[conn->getFD()] = conn;
  FD_SET(conn->getFD(), &master_set);
  if (max_fd < conn->getFD()) {
    max_fd = conn->getFD();
  }
}

void Network::removeConnection( int fd )
{
  DEBUG("Removing a file descriptor %d", fd);
  FD_CLR(fd, &master_set);
  connections.erase(connections.find(fd));
  if (max_fd == fd ) {
    DEBUG("Changing max_fd");
    max_fd = 0;
    ConnMap::iterator itcurr, itend;
    itend = connections.end();
    for (itcurr = connections.begin(); itcurr != itend; itcurr++) {
      if (max_fd < (*itcurr).first)
        max_fd = (*itcurr).first;
    }

  }
}

void Network::addToWriteQueue(Connection::Ptr conn){
  writequeue[conn->getFD()] = conn;
}

void Network::addTimer(TimerCallback callback){
  timers.push(callback);
}

void Network::start()
{
  if (active == true) {
    WARNING("Network already running");
    return;
  }

  if(Game::getGame()->isLoaded()){
    INFO("Starting Network");


    uint32_t numsocks = 0;
    TcpSocket::Ptr listensocket( new TcpSocket() );
    listensocket->openListen(Settings::getSettings()->get("tp_addr"), Settings::getSettings()->get("tp_port"));
    if(listensocket->getStatus() != Connection::DISCONNECTED){
      addConnection(listensocket);
      numsocks++;
      advertiser->addService("tp", listensocket->getPort());
    }else{
      WARNING("Could not listen on TP (tcp) socket");
    }
    if(Settings::getSettings()->get("http") == "yes"){
      HttpSocket::Ptr httpsocket( new HttpSocket() );
      httpsocket->openListen(Settings::getSettings()->get("http_addr"), Settings::getSettings()->get("http_port"));
      if(httpsocket->getStatus() != Connection::DISCONNECTED){
        addConnection(httpsocket);
        numsocks++;
        advertiser->addService("tp+http", httpsocket->getPort());
      }else{
        WARNING("Could not listen on HTTP (http tunneling) socket");
      }
    }else{
      INFO("Not configured to start http socket");
    }
#ifdef HAVE_LIBGNUTLS
    if(Settings::getSettings()->get("tps") == "yes"){
      TlsSocket::Ptr secsocket( new TlsSocket() );
      secsocket->openListen(Settings::getSettings()->get("tps_addr"), Settings::getSettings()->get("tps_port"));
      if(secsocket->getStatus() != Connection::DISCONNECTED){
        addConnection(secsocket);
        numsocks++;
        advertiser->addService("tps", secsocket->getPort());
      }else{
        WARNING("Could not listen on TPS (tls) socket");
      }
    }else{
      INFO("Not configured to start tps socket");
    }
    if(Settings::getSettings()->get("https") == "yes"){
      HttpsSocket::Ptr secsocket( new HttpsSocket() );
      secsocket->openListen(Settings::getSettings()->get("https_addr"), Settings::getSettings()->get("https_port"));
      if(secsocket->getStatus() != Connection::DISCONNECTED){
        addConnection(secsocket);
        numsocks++;
        advertiser->addService("tp+https", secsocket->getPort());
      }else{
        WARNING("Could not listen on HTTPS (https tunneling) socket");
      }
    }else{
      INFO("Not configured to start https socket");
    }
#endif
    if(numsocks != 0){
      INFO("Started network with %d listeners", numsocks);
      active = true;
    }

    advertiser->publish();

  }else{
    WARNING("Not starting network, game not yet loaded");
  }

}


void Network::stop()
{
  if (active) {
    INFO("Stopping Network");

    ConnMap::iterator itcurr = connections.begin();
    while (itcurr != connections.end()) {
      if ( itcurr->second->getType() == Connection::PLAYER || itcurr->second->getType() == Connection::LISTEN )
        removeConnection(itcurr->first);
      ++itcurr;
    }
    advertiser->unpublish();

    active = false;
  } else {
    WARNING("Network already stopped");
  }
}

bool Network::isStarted() const{
  return active;
}

void Network::adminStart(){
  if(Settings::getSettings()->get("admin_tcp") == "yes"){
    AdminTcpSocket::Ptr admintcpsocket( new AdminTcpSocket() );
    admintcpsocket->openListen(Settings::getSettings()->get("admin_tcp_addr"), Settings::getSettings()->get("admin_tcp_port"));
    if(admintcpsocket->getStatus() != Connection::DISCONNECTED){
      addConnection(admintcpsocket);
    }else{
      WARNING("Could not listen on admin TCP socket");
    }
  }else{
    INFO("Not configured to start admin TCP socket");
  }
}

void Network::adminStop(){
  ConnMap::iterator itcurr = connections.begin();
  while (itcurr != connections.end()) {
    if ( itcurr->second->getType() == Connection::ADMIN || 
         itcurr->second->getType() == Connection::LISTENADMIN )
      removeConnection(itcurr->first);
    ++itcurr;
  }
}

void Network::sendToAll(AsyncFrame* aframe){
  ConnMap::iterator itcurr;
  for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
    PlayerConnection::Ptr currConn = boost::dynamic_pointer_cast<PlayerConnection>(itcurr->second);
    if(currConn != NULL && currConn->getStatus() == Connection::READY){
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

Advertiser::Ptr Network::getAdvertiser() const{
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
    for(ConnMap::iterator itcurr = writequeue.begin();
        itcurr != writequeue.end(); ++itcurr){
      FD_SET(itcurr->first, &write_set);
    }

    cur_set = master_set;

    if (select(max_fd + 1, &cur_set, &write_set, NULL, &tv) > 0) {

      for(ConnMap::iterator itcurr = writequeue.begin();
          itcurr != writequeue.end(); ++itcurr){
        if(FD_ISSET(itcurr->first, &write_set)){
          Connection::Ptr conn = itcurr->second;
          writequeue.erase(itcurr);
          conn->processWrite();
          //use select again, don't check rest of list as it has changed.
          break;
        }
      }

      ConnMap::iterator itcurr;
      for (itcurr = connections.begin(); itcurr != connections.end(); itcurr++) {
        Connection::Ptr connection = itcurr->second;
        if (FD_ISSET(itcurr->first, &cur_set)) {
          connection->process();
        }
        if (connection->getStatus() == Connection::DISCONNECTED) {
          INFO("Closed connection %d", connection->getFD());
          removeConnection(itcurr->first);
          //use select again, don't check rest of list as it has changed.
          break;
        }
      }

    }

    //advertiser->poll();

    if(netstat != active && active == false){
      ConnMap::iterator itcurr = connections.begin();
      while (itcurr != connections.end()) {
        removeConnection( itcurr->first );
        ++itcurr;
      }
      DEBUG("Network really stopped");
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

  advertiser = Advertiser::Ptr( new Advertiser() );

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

