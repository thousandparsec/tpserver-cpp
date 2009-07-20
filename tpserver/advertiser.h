/*  Advertiser registration/publishing abstraction for tpserver-cpp
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
#ifndef ADVERTISER_H
#define ADVERTISER_H

#include <tpserver/common.h>
#include <tpserver/publisher.h>

class TimerCallback;

/**
 * Abstraction for registration and publishing of services provided
 * @author Lee Begg <llnz@paradise.net.nz>
 */
class Advertiser{
public:
  /// Typedef for shared pointer
  typedef boost::shared_ptr< Advertiser > Ptr;

  /// Typedef for map of services
  typedef std::map<std::string, uint16_t> ServiceMap;
  
  Advertiser();

  ~Advertiser();
  
  void addService(const std::string &name, uint16_t port);
  void removeService(const std::string &name);
  void removeAll();
  
  ServiceMap getServices();
  
  void publish();
  void unpublish();

  void updatePublishers();

private:
  /// Typedef for publisher set
  typedef std::set<Publisher::Ptr> PublisherSet;

  void settingChanged(const std::string& skey, const std::string& value);
  void metaserverWarning();

  ServiceMap services;
  PublisherSet publishers;

  bool publishing;

  TimerCallback *metaserver_warning;
};

#endif
