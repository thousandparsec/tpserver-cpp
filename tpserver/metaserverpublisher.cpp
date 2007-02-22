/*  Metaserver Publisher for tpserver-cpp
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

#include <time.h>

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
#include "net.h"
#include "timercallback.h"
#include "metaserverconnection.h"

#include "metaserverpublisher.h"

MetaserverPublisher::MetaserverPublisher(Advertiser* ad) : Publisher(ad), lastpublishtime(0), needtoupdate(true), key(), timer(NULL), errorcount(0){
  Settings* settings = Settings::getSettings();
  settings->setCallback("metaserver_fake_ip", SettingsCallback(this, &MetaserverPublisher::metaserverSettingChanged));
  settings->setCallback("metaserver_fake_dns", SettingsCallback(this, &MetaserverPublisher::metaserverSettingChanged));
  settings->setCallback("metaserver_address", SettingsCallback(this, &MetaserverPublisher::metaserverSettingChanged));
  settings->setCallback("metaserver_port", SettingsCallback(this, &MetaserverPublisher::metaserverSettingChanged));
  lastpublishtime = time(NULL) - 600;
  setTimer();
}


MetaserverPublisher::~MetaserverPublisher(){
  Settings* settings = Settings::getSettings();
  settings->removeCallback("metaserver_fake_ip");
  settings->removeCallback("metaserver_fake_dns");
  settings->removeCallback("metaserver_address");
  settings->removeCallback("metaserver_port");
  if(timer != NULL){
    timer->setValid(false);
    delete timer;
  }
}

void MetaserverPublisher::poll(){
    // create MetaserverConnection and send info.
  MetaserverConnection* msc = new MetaserverConnection(advertiser, this);
  if(msc->sendUpdate()){
    Network::getNetwork()->addConnection(msc);
    lastpublishtime = time(NULL);
    needtoupdate = false;
    errorcount = 0;
  }else{
    errorcount++;
    needtoupdate = true;
    if(errorcount < 5){
      lastpublishtime = time(NULL) - 60; //wait a minute before trying again
    }else{
      errorcount = 0;
      lastpublishtime = time(NULL) + 600; //wait 12 minutes before trying again
      Logger::getLogger()->warning("Not trying metaserver for 12 minutes");
    }
    delete msc;
  }
  setTimer();
}

void MetaserverPublisher::update(){
  needtoupdate = true;
  setTimer();
}

std::string MetaserverPublisher::getKey(){
  return key;
}

void MetaserverPublisher::metaserverSettingChanged(const std::string& skey, const std::string& value){
  update();
}

void MetaserverPublisher::setTimer(){
  if(timer != NULL){
    timer->setValid(false);
    delete timer;
  }
  uint64_t seconds = lastpublishtime;
  if(needtoupdate){
    seconds += 120;
  }else{
    uint32_t updatetimer = atoi(Settings::getSettings()->get("metaserver_interval").c_str());
    if(updatetimer == 0) updatetimer = 360;
    if(updatetimer < 120) updatetimer = 120;
    if(updatetimer > 600) updatetimer = 600;
    seconds += updatetimer;
  }
  seconds -= time(NULL);
  timer = new TimerCallback(this, &MetaserverPublisher::poll, seconds);
  Network::getNetwork()->addTimer(*timer);
}
