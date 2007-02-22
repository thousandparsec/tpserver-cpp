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
#ifndef METASERVERPUBLISHER_H
#define METASERVERPUBLISHER_H

#include <stdint.h>
#include <string>

#include "publisher.h"

class TimerCallback;

/**
Metaserver Publisher for tpserver-cpp
 
	@author Lee Begg <llnz@paradise.net.nz>
*/
class MetaserverPublisher : public Publisher{
public:
  MetaserverPublisher(Advertiser* ad);

  virtual ~MetaserverPublisher();
  
  void update();

  std::string getKey();
  

private:
  void poll();
  void setTimer();
  void metaserverSettingChanged(const std::string& skey, const std::string& value);
  uint64_t lastpublishtime;
  bool needtoupdate;
  std::string key;
  TimerCallback* timer;
  int errorcount;

};

#endif
