#ifndef CONNECTION_H
#define CONNECTION_H
/*  Network Connection class
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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

class Frame;
class Player;

// this header should be removed
#include "frame.h"

class Connection {

      public:

	Connection();
	Connection(Connection & rhs);
	Connection(int fd);
	~Connection();
	Connection operator=(Connection & rhs);

	int getFD();
	void setFD(int fd);

	void process();
	void close();
	void sendFrame(Frame * frame);

	Frame* createFrame(Frame* oldframe = NULL);

	int getStatus();

	FrameVersion getProtocolVersion();

      private:

	void verCheck();
	void login();

	void inGameFrame();

	bool readFrame(Frame * recvframe);

	int sockfd;
	Player *player;
	int status;

	FrameVersion version;

};

#endif
