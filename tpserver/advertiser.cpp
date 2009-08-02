/*  Advertiser registration/publishing abstraction for tpserver-cpp
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
#include <boost/bind.hpp>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logging.h"
#include "settings.h"
#include "timercallback.h"
#include "net.h"

#ifdef HAVE_AVAHI
#include "avahi.h"
#endif
#include "metaserverpublisher.h"

#include "advertiser.h"


Advertiser::Advertiser() : publishing(false) {
  Settings* settings = Settings::getSettings();
  settings->setCallback("game_name",         boost::bind( &Advertiser::settingChanged, this, _1, _2 ));
  settings->setCallback("game_shortname",    boost::bind( &Advertiser::settingChanged, this, _1, _2 ));
  settings->setCallback("game_comment",      boost::bind( &Advertiser::settingChanged, this, _1, _2 ));
  settings->setCallback("admin_email",       boost::bind( &Advertiser::settingChanged, this, _1, _2 ));
  settings->setCallback("metaserver_enable", boost::bind( &Advertiser::settingChanged, this, _1, _2 ));
}


Advertiser::~Advertiser(){
  unpublish();
  Settings* settings = Settings::getSettings();
  settings->removeCallback("game_name");
  settings->removeCallback("game_shortname");
  settings->removeCallback("game_comment");
  settings->removeCallback("admin_email");
  settings->removeCallback("metaserver_enable");
}

void Advertiser::publish(){
#ifdef HAVE_AVAHI
  try{
    publishers.insert( Publisher::Ptr( new Avahi() ) );
  }catch(std::exception e){
    // do nothing, maybe warn of no mdns-sd
      WARNING("Failed to start Avahi");
  }
#endif
  if(Settings::getSettings()->get("metaserver_enable") == "yes"){
    publishers.insert( Publisher::Ptr( new MetaserverPublisher() ));
  }else{
    WARNING("Metaserver updates disabled, set metaserver_enable to \"yes\" to enable them");
    if(Settings::getSettings()->get("metaserver_enable") != "no"){
      metaserver_warning.reset( new TimerCallback( boost::bind( &Advertiser::metaserverWarning, this ), 600) );
      Network::getNetwork()->addTimer( metaserver_warning );
    }
  }
  publishing = true;
  updatePublishers();
}

void Advertiser::unpublish(){
  publishing = false;
  if(metaserver_warning){
    metaserver_warning->invalidate();
    metaserver_warning.reset();
  }
  publishers.clear();
}

void Advertiser::addService(const std::string &name, uint16_t port){
  services[name] = port;
  updatePublishers();
}

void Advertiser::removeService(const std::string &name){
  services.erase(name);
  updatePublishers();
}

void Advertiser::removeAll(){
  services.clear();
  updatePublishers();
}

Advertiser::ServiceMap Advertiser::getServices(){
  return services;
}

void Advertiser::updatePublishers(){
  if(publishing){
    std::for_each( publishers.begin(), publishers.end(), boost::mem_fn( &Publisher::update ) );
  }
}

void Advertiser::settingChanged(const std::string& skey, const std::string& value){
  if(skey == "metaserver_enable"){
    if(publishing){
      PublisherSet::iterator meta = std::find_if( publishers.begin(), publishers.end(), boost::mem_fn( &Publisher::isMetaserver ) );
      if(value != "yes"){
        if ( meta != publishers.end() )
        {
          publishers.erase(meta);
        }
      }else{
        if(metaserver_warning){
          metaserver_warning->invalidate();
          metaserver_warning.reset();
        }
        if( meta == publishers.end() ){
          publishers.insert( Publisher::Ptr( new MetaserverPublisher() ));
        }
      }
    }
  }else{
    updatePublishers();
  }
}

void Advertiser::metaserverWarning(){
  WARNING("Metaserver updates disabled, set metaserver_enable to \"yes\" to enable them");
  metaserver_warning.reset( new TimerCallback( boost::bind( &Advertiser::metaserverWarning, this ), 600) );
  Network::getNetwork()->addTimer(metaserver_warning);
}
