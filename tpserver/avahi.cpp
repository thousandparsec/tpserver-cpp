/*  Avahi mDNS-SD abstraction for tpserver-cpp
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

#include <avahi-client/client.h>

#include <avahi-client/publish.h>
#include <avahi-common/alternative.h>

#include <avahi-common/watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include <avahi-common/timeval.h>

#include <time.h>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifndef VERSION
#define VERSION "0.0.0"
#endif
#endif

#include "advertiser.h"
#include "logging.h"
#include "settings.h"
#include "settingscallback.h"
#include "game.h"
#include "ruleset.h"
#include "objectmanager.h"
#include "playermanager.h"
#include "net.h"
#include "timercallback.h"
#include "net.h"
#include "connection.h"
#include "turntimer.h"

#include "avahi.h"

//client_callback
void client_callback(AvahiClient *c, AvahiClientState state, void * userdata) {
  Avahi* avahi = static_cast<Avahi*>(userdata);
  
  assert(c);
  if(avahi->client == NULL)
    avahi->client = c;

        /* Called whenever the client or server state changes */

        switch (state) {
        case AVAHI_CLIENT_S_RUNNING:
        
            /* The server has startup successfully and registered its host
            * name on the network, so it's time to create our services */
            if (!avahi->group)
                avahi->createServices();
            break;

        case AVAHI_CLIENT_S_COLLISION:
        
            /* Let's drop our registered services. When the server is back
            * in AVAHI_SERVER_RUNNING state we will register them
            * again with the new host name. */
            if (avahi->group)
                avahi_entry_group_reset(avahi->group);
            break;
            
        case AVAHI_CLIENT_FAILURE:
            
            Logger::getLogger()->warning("Client failure: %s", avahi_strerror(avahi_client_errno(c)));
//            avahi_simple_poll_quit(avahi->simple_poll);
            avahi->reset();
            
            break;

        case AVAHI_CLIENT_CONNECTING:
        case AVAHI_CLIENT_S_REGISTERING:
            ;
    }
}

void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {
    //assert(g == group);
    Avahi* avahi = static_cast<Avahi*>(userdata);
    /* Called whenever the entry group state changes */

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED :
            /* The entry group has been established successfully */
            Logger::getLogger()->debug("Service '%s' successfully established.", avahi->name);
            break;

            case AVAHI_ENTRY_GROUP_COLLISION : {
                char *n;
            
                /* A service name collision happened. Let's pick a new name */
                n = avahi_alternative_service_name(avahi->name);
                avahi_free(avahi->name);
                avahi->name = n;
            
                Logger::getLogger()->warning("Service name collision, renaming service to '%s'", avahi->name);
            
                /* And recreate the services */
                avahi->createServices();
                break;
            }

        case AVAHI_ENTRY_GROUP_FAILURE :

            /* Some kind of failure happened while we were registering our services */
//            avahi_simple_poll_quit(avahi->simple_poll);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

// declarations for avahipoll api
class AvahiWatch : public Connection{
 public:
  AvahiWatch(int fd, AvahiWatchCallback cb, void* ud): Connection(), callback(cb), watchedEvents(), happenedEvents(), userdata(ud){
    sockfd = fd;
    status = 1;
    Network::getNetwork()->addConnection(this);
  }
  
  ~AvahiWatch(){
    Network::getNetwork()->removeConnection(this);
  }
  
  void process(){
    if(watchedEvents == AVAHI_WATCH_IN){
      happenedEvents = AVAHI_WATCH_IN;
      callback(this, sockfd, happenedEvents, userdata);
    }
  }
  
  void processWrite(){
    if(watchedEvents == AVAHI_WATCH_OUT){
      happenedEvents = AVAHI_WATCH_OUT;
      callback(this, sockfd, happenedEvents, userdata);
      Network::getNetwork()->addToWriteQueue(this);
    }
  }
  
  void update(AvahiWatchEvent e){
    watchedEvents = e;
    happenedEvents = (AvahiWatchEvent)0;
    if(watchedEvents == AVAHI_WATCH_OUT){
      Network::getNetwork()->addToWriteQueue(this);
    }
    if(watchedEvents == AVAHI_WATCH_ERR || watchedEvents == AVAHI_WATCH_HUP){
      Logger::getLogger()->debug("AvahiWatch update withe ERR or HUP event set %x", watchedEvents);
    }
    Logger::getLogger()->debug("AvahiWatch event mask %x", watchedEvents);
  }
  
  AvahiWatchEvent getEvents(){
    return happenedEvents;
  }
  
  private:
    AvahiWatchCallback callback;
    AvahiWatchEvent watchedEvents;
    AvahiWatchEvent happenedEvents;
    void* userdata;
};

class AvahiTimeout{
  public:
    AvahiTimeout(AvahiTimeoutCallback cb, void* ud):  timer(NULL), callback(cb), userdata(ud){
    }
    
    ~AvahiTimeout(){
      if(timer != NULL){
        timer->setValid(false);
        delete timer;
      }
    }
    
    void setTimer(uint64_t sec){
      if(timer != NULL){
        timer->setValid(false);
        delete timer;
      }
      timer = new TimerCallback(this, &AvahiTimeout::timeout, sec);
      Network::getNetwork()->addTimer(*timer);
    }
    
    void disableTimer(){
      if(timer != NULL){
        timer->setValid(false);
        delete timer;
      }
      timer = NULL;
    }
    
    void timeout(){
      callback(this, userdata);
    }
    
  private:
    TimerCallback* timer;
    AvahiTimeoutCallback callback;
    void* userdata;
};

AvahiWatch* watch_new(const AvahiPoll* api, int fd, AvahiWatchEvent e, AvahiWatchCallback callback, void *userdata){
  AvahiWatch* watch = new AvahiWatch(fd, callback, userdata);
  watch->update(e);
  return watch;
}

void watch_update(AvahiWatch* w, AvahiWatchEvent event){
  w->update(event);
}

AvahiWatchEvent watch_get_events(AvahiWatch *w){
  return w->getEvents();
}

void watch_free(AvahiWatch *w){
  delete w;
}

AvahiTimeout* timeout_new(const AvahiPoll* api, const struct timeval* tv, AvahiTimeoutCallback callback, void* userdata){
  AvahiTimeout* t = new AvahiTimeout(callback, userdata);
  if(tv != NULL)
    t->setTimer(tv->tv_sec - time(NULL));
  return t;
}

void timeout_update(AvahiTimeout* t, const struct timeval* tv){
  if(tv != NULL){
    t->setTimer(tv->tv_sec - time(NULL));
  }else{
    t->disableTimer();
  }
}

void timeout_free(AvahiTimeout *t){
  delete t;
}

Avahi::Avahi(Advertiser* ad) : Publisher(ad), pollapi(NULL), group(NULL), client(NULL), name(NULL), resetTimer(NULL){
  
  std::string tname = Settings::getSettings()->get("game_name");
  if(tname.empty())
    tname = "Tpserver-cpp";
  name = avahi_strdup(tname.c_str());
  
  pollapi = new AvahiPoll();
  pollapi->userdata = this;
  pollapi->watch_new = watch_new;
  pollapi->watch_update = watch_update;
  pollapi->watch_get_events = watch_get_events;
  pollapi->watch_free = watch_free;
  pollapi->timeout_new = timeout_new;
  pollapi->timeout_update = timeout_update;
  pollapi->timeout_free = timeout_free;
  
  reset();

}


Avahi::~Avahi(){
    if(group)
        avahi_entry_group_free(group);
    
  if (client)
        avahi_client_free(client);

  if (pollapi)
      delete pollapi;
  
  if(resetTimer != NULL){
      resetTimer->setValid(false);
      delete resetTimer;
  }
  
}

void Avahi::update(){
  std::string tname = Settings::getSettings()->get("game_name");
  if(tname.empty())
    tname = "Tpserver-cpp";
  avahi_free(name);
  name = avahi_strdup(tname.c_str());
  createServices();
}

void Avahi::createServices(){
  int ret;
  std::map<std::string, uint16_t> services = advertiser->getServices();
  
  // Don't export tp+http and tp+https unless they are the only ones
  std::map<std::string, uint16_t> nservices = services;
  nservices.erase("tp+http");
  nservices.erase("tp+https");
  if(!nservices.empty()){
    services = nservices;
  }
  
  AvahiStringList * txtfields = avahi_string_list_new(
      "server=" VERSION,
      "sertype=tpserver-cpp", 
      "tp=0.3,0.4",
      NULL);
  txtfields = avahi_string_list_add(txtfields, (std::string("rule=") + Game::getGame()->getRuleset()->getName()).c_str());
  txtfields = avahi_string_list_add(txtfields,(std::string("rulever=") + Game::getGame()->getRuleset()->getVersion()).c_str());
  TurnTimer* turntimer = Game::getGame()->getTurnTimer();
  if(turntimer != NULL){
    txtfields = avahi_string_list_add_printf(txtfields, "next=%ld", turntimer->secondsToEOT() + time(NULL));
    txtfields = avahi_string_list_add_printf(txtfields, "prd=%d", turntimer->getTurnLength());
  }
  txtfields = avahi_string_list_add_printf(txtfields, "objs=%d", Game::getGame()->getObjectManager()->getNumObjects());
  txtfields = avahi_string_list_add_printf(txtfields, "plys=%d", Game::getGame()->getPlayerManager()->getNumPlayers());
  txtfields = avahi_string_list_add_printf(txtfields, "turn=%d", Game::getGame()->getTurnNumber());
  
  
  Settings* settings = Settings::getSettings();
  
  if(!(settings->get("admin_email").empty())){
    txtfields = avahi_string_list_add(txtfields, (std::string("admin=") + settings->get("admin_email")).c_str());
  }
  if(!(settings->get("game_comment").empty())){
    std::string comment = settings->get("game_comment");
    txtfields = avahi_string_list_add(txtfields, (std::string("cmt=") + comment).c_str());
  }
  if(!(settings->get("game_shortname").empty())){
    std::string shortname = settings->get("game_shortname");
    txtfields = avahi_string_list_add(txtfields, (std::string("sn=") + shortname).c_str());
  }else{
    txtfields = avahi_string_list_add(txtfields, "sn=tp");
  }
  
  /* If this is the first time we're called, let's create a new entry group */
  if (!group)
    if (!(group = avahi_entry_group_new(client, entry_group_callback, this))) {
      Logger::getLogger()->warning("avahi_entry_group_new() failed: %s", avahi_strerror(avahi_client_errno(client)));
      goto fail;
    }
  
  Logger::getLogger()->debug("Adding service '%s'", name);
  avahi_entry_group_reset(group);
  
  for(std::map<std::string, uint16_t>::iterator itcurr = services.begin();
      itcurr != services.end(); ++itcurr){
        std::string sertype = itcurr->first;
        size_t pos;
        if((pos = sertype.find('+')) != sertype.npos){
          sertype.replace(pos,1, "-");
        }
        std::string servicename = std::string("_") + sertype + "._tcp";
    // after the port, there is a NULL terminated list of strings for the TXT field
    if ((ret = avahi_entry_group_add_service_strlst(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
         (AvahiPublishFlags)0, name, servicename.c_str(), NULL, NULL, itcurr->second, txtfields)) < 0) {
        Logger::getLogger()->warning("Failed to add %s service: %s", servicename.c_str(), avahi_strerror(ret));
        goto fail;
    }
  }

  avahi_string_list_free(txtfields);
  
  
  /* Tell the server to register the service */
  if (!avahi_entry_group_is_empty(group) && (ret = avahi_entry_group_commit(group)) < 0) {
      Logger::getLogger()->warning("Failed to commit entry_group: %s", avahi_strerror(ret));
      goto fail;
  }

  return;

fail:
//  avahi_simple_poll_quit(simple_poll);
    ;
}

void Avahi::reset(){
    
    if(client != NULL){
        if(group != NULL){
            avahi_entry_group_free(group);
            group = NULL;
        }
        avahi_client_free(client);
    }
    int error;
    
    /* Allocate a new client */
    client = avahi_client_new(pollapi, AVAHI_CLIENT_NO_FAIL, client_callback, this, &error);
    
    /* Check wether creating the client object succeeded */
    if (!client) {
        Logger::getLogger()->warning("Failed to create avahi client: %s", avahi_strerror(error));
        resetTimer = new TimerCallback(this, &Avahi::reset, 60);
        Network::getNetwork()->addTimer(*resetTimer);
    }
}
