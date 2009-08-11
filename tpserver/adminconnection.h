#ifndef ADMINCONNECTION_H
#define ADMINCONNECTION_H
/*  Admin Connection class
 *
 *  Copyright (C) 2008 Aaron Mavrinac and the Thousand Parsec Project
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

#include <tpserver/tcpconnection.h>

class AdminLogger;

class AdminConnection: public TcpConnection {
 public:
   typedef boost::shared_ptr< AdminConnection > Ptr;
  AdminConnection(int fd);
  virtual ~AdminConnection();
  
 protected:

  virtual void processLogin();
  virtual void processNormalFrame();

  void processDescribeCommand(InputFrame * frame);
  void processGetCommandTypes(InputFrame * frame);
  void processCommand(InputFrame * frame);

  AdminLogger* logsink;
  int logextid;

};

#endif
