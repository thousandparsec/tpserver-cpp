/*  Metaserver Connection for tpserver-cpp with ipv4 and ipv6 support
 *
 * This connection is a little odd, because it is a client connection
 * to the metaserver, not a server connection of this server.
 *
 *  Copyright (C) 2006,2007  Lee Begg and the Thousand Parsec Project
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
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sstream>

#include <iostream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifndef VERSION
#define VERSION 0.0.0
#endif

#include "logging.h"
#include "advertiser.h"
#include "metaserverpublisher.h"
#include "net.h"
#include "game.h"
#include "ruleset.h"
#include "settings.h"
#include "playermanager.h"
#include "objectmanager.h"

#include "metaserverconnection.h"

MetaserverConnection::MetaserverConnection(Advertiser* ad, MetaserverPublisher* pub) : Connection(), advertiser(ad), publisher(pub){

}

MetaserverConnection::~MetaserverConnection(){
  if(status != 0)
    close(sockfd);
  status = 0;
}

bool MetaserverConnection::sendUpdate(){

  status = 1;
  
  Settings* settings = Settings::getSettings();

  std::string host = settings->get("metaserver_address");
  std::string port = settings->get("metaserver_port");
  
  if(host.length() == 0){
    host = "metaserver.thousandparsec.net";
  }
  
  if(port.length() == 0){
    port = "80";
  }

#ifdef HAVE_IPV6

  struct addrinfo hints, *res, *ressave;

  memset(&hints, 0, sizeof(struct addrinfo));
  
  hints.ai_family   = PF_UNSPEC; 
  hints.ai_socktype = SOCK_STREAM;

  int n = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
  
  if (n < 0) {
    Logger::getLogger()->error("Metaserver: Could not getaddrinfo, %s", gai_strerror(n));
    status = 0;
    return false;
  }
  
  ressave=res;

  sockfd=-1;
  while (res) {
    sockfd = socket(res->ai_family,
                    res->ai_socktype,
                    res->ai_protocol);

    if (!(sockfd < 0)) {
//             fcntl(sockfd, F_SETFL, O_NONBLOCK);
//             int yes = 1;
//             setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
        break;
      
      close(sockfd);
      sockfd=-1;
    }
    res = res->ai_next;
  }

  freeaddrinfo(ressave);
#else
    
    // IPv4 only
  
  struct sockaddr_in sin;
  struct hostent *phe;
  struct servent *pse;
  
  memset(&sin, 0, sizeof(sin));
  
  sin.sin_family=AF_INET;
  
  if ( pse = getservbyname(port.c_str(), "tcp") ) {
    sin.sin_port = pse->s_port;
    
  } else if ((sin.sin_port = htons((u_short)atoi(port.c_str())))==0) {
    fprintf(stderr, "ipv4_only_connect:: could not get service=[%s]\n",
            service);
    status = 0;
    return false;
  }
  
  if (phe = gethostbyname(host.c_str())) {
    memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    
  } else if ( (sin.sin_addr.s_addr = inet_addr(host.c_str())) == 
              INADDR_NONE) {
    fprintf(stderr, "ipv4_only_connect:: could not get host=[%s]\n", hostname);
    status = 0;
    return false;
  }
  
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {  
    fprintf(stderr, "ipv4_only_connect:: could not open socket\n");
    status = 0;
    return false;
  }
  
// 	fcntl(sockfd, F_SETFL, O_NONBLOCK);
//         int yes = 1;
//         setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (connect(sockfd,(struct sockaddr *)&sin, sizeof(sin)) < 0) {
    fprintf(stderr, "ipv4_only_connect:: could not connect to host=[%s]\n", hostname);
    status = 0;
    return false;
  }

#endif
  if (sockfd == -1){
    Logger::getLogger()->warning("Could not create Metaserver Connection socket");
    status = 0;
    return false;
  }
  
  std::string localname;
  std::string localip;
  
#ifdef HAVE_IPV6

  char* myname = new char[NI_MAXHOST];
  char* straddr = new char[INET6_ADDRSTRLEN];
  
  sockaddr_storage laddr;
  unsigned int lalen = sizeof(laddr);
  getsockname(sockfd, (sockaddr*)(&laddr), &lalen);
  
  if(laddr.ss_family == AF_INET6){
    inet_ntop(AF_INET6, &(((sockaddr_in6*)(&laddr))->sin6_addr), straddr,
              INET6_ADDRSTRLEN);
  }else{
    inet_ntop(AF_INET, &(((sockaddr_in*)(&laddr))->sin_addr), straddr,
              INET6_ADDRSTRLEN);
  }

  getnameinfo((struct sockaddr *)&laddr, lalen,
              myname, NI_MAXHOST,
              NULL, 0,
              0);

  localname = myname;
  delete[] myname;
  localip = straddr;
  delete[] straddr;
  
#else
  
  sockaddr_in laddr;
  unsigned int lalen = sizeof(laddr);
  
  char* myname = new char[NI_MAXHOST];
  char* myip;
  
  gethostname(myname, NI_MAXHOST);
  getsocketname(sockfd, (sockaddr*)(&laddr), &lalen);
  myip = inet_ntoa(laddr.sin_addr);
  
  localname = myname;
  delete[] myname;
  localip = myip;
  
#endif
  
  if(settings->get("metaserver_fake_ip") != ""){
    localip = settings->get("metaserver_fake_ip");
  }
  if(settings->get("metaserver_fake_dns") != ""){
    localname = settings->get("metaserver_fake_dns");
  }

  std::string tname = settings->get("server_name");
  if(tname.empty())
    tname = "Tpserver-cpp";
  
  size_t pos;
  while((pos = tname.find(' ')) != tname.npos){
    tname.replace(pos,1, "%20");
  }

  std::ostringstream formater;
  formater << "GET /?action=update&sertype=tpserver-cpp&server=" VERSION;
  formater << "&tp=0.3,0.2&key=" << publisher->getKey();
  formater << "&name=" << tname;
  formater << "&rule=" << Game::getGame()->getRuleset()->getName();
  formater << "&rulever=" << Game::getGame()->getRuleset()->getVersion();
  formater << "&turn=" << (Game::getGame()->secondsToEOT() + time(NULL));
  formater << "&objs=" << (Game::getGame()->getObjectManager()->getNumObjects());
  formater << "&plys=" << (Game::getGame()->getPlayerManager()->getNumPlayers());
  if(!(settings->get("admin_email").empty())){
    formater << "&admin=" << settings->get("admin_email");
  }
  if(!(settings->get("game_comment").empty())){
    std::string comment = settings->get("game_comment");
    while((pos = comment.find(' ')) != comment.npos){
      comment.replace(pos,1, "%20");
    }
    formater << "&cmt=" << comment;
  }
  
  int servicenumber = 0;
  std::map<std::string, uint16_t> services = advertiser->getServices();
  for(std::map<std::string, uint16_t>::iterator itcurr = services.begin();
      itcurr != services.end(); ++itcurr){
    formater << "&type" << servicenumber << "=" << itcurr->first;
    formater << "&port" << servicenumber << "=" << itcurr->second;
    formater << "&dns" << servicenumber << "=" << localname;
    formater << "&ip" << servicenumber << "=" << localip;
    servicenumber++;
  }

  formater << " HTTP/1.0\r\nUser-agent: tpserver-cpp/" VERSION "\r\n";
  formater << "Host: " << host << "\r\n";
  formater << "\r\n";
  
  Logger::getLogger()->debug("Sending update info to metaserver");
  
  std::string request = formater.str();
  std::cout << "This is the metaserver update request" << std::endl;
  std::cout << request << std::endl;
  
  send(sockfd, request.c_str(), request.length(), 0);
  
  status = 1;
  
  return true;
}

void MetaserverConnection::process(){
  Logger::getLogger()->debug("Data back from metaserver");

  char* buffer = new char[1024];
  int rlen = recv(sockfd, buffer, 1024, 0);
  
  if(rlen == 0){
    Logger::getLogger()->debug("Metaserver disconnected");
    close(sockfd);
    status = 0;
    delete[] buffer;
    std::cout << response << std::endl;
    return;
  }
  
  response.append(buffer, rlen);
  
  delete[] buffer;

}

