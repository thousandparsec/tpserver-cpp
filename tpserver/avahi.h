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
#ifndef AVAHI_H
#define AVAHI_H

#include <map>
#include <string>
#include <avahi-client/client.h>

#include "publisher.h"

struct AvahiEntryGroup;

class TimerCallback;

/**
Avahi mDNS-SD implementation for tpserver-cpp
 
	@author Lee Begg <llnz@paradise.net.nz>
*/
class Avahi : public Publisher{
public:
  Avahi(Advertiser* ad);

  virtual ~Avahi();
  
  void update();

  //callbacks
  friend void client_callback(AvahiClient *c, AvahiClientState state, void * userdata);
  friend void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata);

private:
  void createServices();
  void reset();
  
  AvahiPoll *pollapi;
  AvahiEntryGroup *group;
  AvahiClient *client;
  char* name;
  
  TimerCallback* resetTimer;

};

#endif
