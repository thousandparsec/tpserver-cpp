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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// #include "logging.h"
#include "settings.h"
#include "settingscallback.h"
#include "publisher.h"

#ifdef HAVE_AVAHI
#include "avahi.h"
#endif
#include "metaserverpublisher.h"

#include "advertiser.h"


Advertiser::Advertiser() : services(), publishers(){
  Settings* settings = Settings::getSettings();
  settings->setCallback("server_name", SettingsCallback(this, &Advertiser::settingChanged));
  settings->setCallback("game_comment", SettingsCallback(this, &Advertiser::settingChanged));
  settings->setCallback("admin_email", SettingsCallback(this, &Advertiser::settingChanged));
}


Advertiser::~Advertiser(){
  unpublish();
  Settings* settings = Settings::getSettings();
  settings->removeCallback("server_name");
  settings->removeCallback("game_comment");
  settings->removeCallback("admin_email");
}

void Advertiser::publish(){
#ifdef HAVE_AVAHI
  try{
    publishers.insert(new Avahi(this));
  }catch(std::exception e){
    // do nothing, maybe warn of no mdns-sd
  }
#endif
  publishers.insert(new MetaserverPublisher(this));
  updatePublishers();
}

void Advertiser::unpublish(){
  for(std::set<Publisher*>::iterator itcurr = publishers.begin(); itcurr != publishers.end(); ++itcurr){
    delete (*itcurr);
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

std::map<std::string, uint16_t> Advertiser::getServices(){
  return services;
}

void Advertiser::updatePublishers(){
  for(std::set<Publisher*>::iterator itcurr = publishers.begin(); itcurr != publishers.end(); ++itcurr){
    (*itcurr)->update();
  }
}

void Advertiser::settingChanged(const std::string& skey, const std::string& value){
  updatePublishers();
}
