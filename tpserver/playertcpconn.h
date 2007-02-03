#ifndef PLAYERTCPCONN_H
#define PLAYERTCPCONN_H
/*  Player TCP Connection class
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

#include <queue>

#include "playerconnection.h"

class PlayerTcpConnection : public PlayerConnection {
 public:
  PlayerTcpConnection();
  PlayerTcpConnection(int fd);
  virtual ~PlayerTcpConnection();
  
  void setFD(int fd);

  void close();
  void sendFrame(Frame * frame);
  void processWrite();
  
 protected:
  void verCheck();
  virtual int32_t verCheckPreChecks();
  virtual int32_t verCheckLastChance();
  
  bool readFrame(Frame * recvframe);
  
  void sendData(const char* data, uint32_t size);
  void sendDataAndClose(const char* data, uint32_t size);
  
  virtual int32_t underlyingRead(char* buff, uint32_t size);
  virtual int32_t underlyingWrite(const char* buff, uint32_t size);
  
  char* rheaderbuff;
  char* rdatabuff;
  uint32_t rbuffused;
  
  char* sbuff;
  uint32_t sbuffused;
  uint32_t sbuffsize;
  std::queue<Frame*> sendqueue;
  
  bool sendandclose;
  
};

#endif
