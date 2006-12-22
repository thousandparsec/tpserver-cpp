/*  Avahi mDNS-SD abstraction for tpserver-cpp
 *
 *  Copyright (C) 2006  Lee Begg and the Thousand Parsec Project
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

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include <avahi-common/timeval.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifndef VERSION
#define VERSION "0.0.0"
#endif
#endif

#include "logging.h"

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
            
            fprintf(stderr, "Client failure: %s\n", avahi_strerror(avahi_client_errno(c)));
            avahi_simple_poll_quit(avahi->simple_poll);
            
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
            fprintf(stderr, "Service '%s' successfully established.\n", avahi->name);
            break;

            case AVAHI_ENTRY_GROUP_COLLISION : {
                char *n;
            
                /* A service name collision happened. Let's pick a new name */
                n = avahi_alternative_service_name(avahi->name);
                avahi_free(avahi->name);
                avahi->name = n;
            
                fprintf(stderr, "Service name collision, renaming service to '%s'\n", avahi->name);
            
                /* And recreate the services */
                avahi->createServices();
                break;
            }

        case AVAHI_ENTRY_GROUP_FAILURE :

            /* Some kind of failure happened while we were registering our services */
            avahi_simple_poll_quit(avahi->simple_poll);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

Avahi::Avahi() : services(), simple_poll(NULL), group(NULL), client(NULL), name(NULL){
  
  name = avahi_strdup("Tpserver-cpp");
  
  /* Allocate main loop object */
  if (!(simple_poll = avahi_simple_poll_new())) {
        Logger::getLogger()->warning("Could not create poll object for avahi");
        throw std::exception();
  }
  
  int error;
  
   /* Allocate a new client */
  client = avahi_client_new(avahi_simple_poll_get(simple_poll), (AvahiClientFlags)0, client_callback, this, &error);
  
  /* Check wether creating the client object succeeded */
  if (!client) {
      Logger::getLogger()->warning("Failed to create client: %s", avahi_strerror(error));
      avahi_simple_poll_free(simple_poll);
      throw std::exception();
  }

}


Avahi::~Avahi(){
  if (client)
        avahi_client_free(client);

  if (simple_poll)
      avahi_simple_poll_free(simple_poll);
}

void Avahi::poll(){
  avahi_simple_poll_iterate(simple_poll, 0);
}

void Avahi::addService(const std::string &name, uint16_t port){
  services[name] = port;
  createServices();
}

void Avahi::removeService(const std::string &name){
  
  services.erase(name);
  createServices();
}

void Avahi::removeAll(){
  
  services.clear();
}

void Avahi::createServices(){
  int ret;

  /* If this is the first time we're called, let's create a new entry group */
  if (!group)
    if (!(group = avahi_entry_group_new(client, entry_group_callback, this))) {
      fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_client_errno(client)));
      goto fail;
    }
  
  fprintf(stderr, "Adding service '%s'\n", name);
  avahi_entry_group_reset(group);
  
  for(std::map<std::string, uint16_t>::iterator itcurr = services.begin();
      itcurr != services.end(); ++itcurr){
        std::string servicename = std::string("_") + itcurr->first + "._tcp";
    // after the port, there is a NULL terminated list of strings for the TXT field
    if ((ret = avahi_entry_group_add_service(group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, (AvahiPublishFlags)0, name, servicename.c_str(), NULL, NULL, itcurr->second, "version=" VERSION, NULL)) < 0) {
        fprintf(stderr, "Failed to add %s service: %s\n", servicename.c_str(), avahi_strerror(ret));
        goto fail;
    }
  }

  
  
  /* Tell the server to register the service */
  if (!avahi_entry_group_is_empty(group) && (ret = avahi_entry_group_commit(group)) < 0) {
      fprintf(stderr, "Failed to commit entry_group: %s\n", avahi_strerror(ret));
      goto fail;
  }

  return;

fail:
  avahi_simple_poll_quit(simple_poll);
}
