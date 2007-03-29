#ifndef PLAYERCONNECTION_H
#define PLAYERCONNECTION_H
/*  Player Connection class
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <stdint.h>

class PlayerAgent;

#include <tpserver/frame.h>

#include <tpserver/connection.h>

class PlayerConnection: public Connection {
  
 public:
  PlayerConnection();
  PlayerConnection(int fd);
  virtual ~PlayerConnection();
  
  void setFD(int fd);
  
  void process();
  virtual void close() = 0;
  virtual void sendFrame(Frame * frame) = 0;
  
  Frame* createFrame(Frame* oldframe = NULL);
  
  FrameVersion getProtocolVersion();
  
 protected:
  
  virtual void verCheck() = 0;
  void login();
  
  void inGameFrame();
  
  virtual bool readFrame(Frame * recvframe) = 0;
  
  PlayerAgent *playeragent;
  
  FrameVersion version;
  uint64_t lastpingtime;
  bool paddingfilter;
};

#endif
