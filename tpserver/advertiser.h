/*  Advertiser registration/publishing abstraction for tpserver-cpp
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
#ifndef ADVERTISER_H
#define ADVERTISER_H

#include <map>
#include <set>
#include <string>

class Publisher;

/**
Abstraction for registration and publishing of services provided
 
	@author Lee Begg <llnz@paradise.net.nz>
*/
class Advertiser{
public:
  Advertiser();

  ~Advertiser();
  
  void addService(const std::string &name, uint16_t port);
  void removeService(const std::string &name);
  void removeAll();
  
  std::map<std::string, uint16_t> getServices();
  
  void publish();
  void poll();
  void unpublish();

  void updatePublishers();

private:
  std::map<std::string, uint16_t> services;
  std::set<Publisher*> publishers;

};

#endif
