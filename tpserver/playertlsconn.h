#ifndef PLAYERTLSCONN_H
#define PLAYERTLSCONN_H
/*  Player TLS Connection class
 *
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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

typedef struct gnutls_session_int* gnutls_session_t;

#include "playertcpconn.h"

class PlayerTlsConnection : public PlayerTcpConnection {
 public:
  PlayerTlsConnection(int fd);
  virtual ~PlayerTlsConnection();

  void close();
  
 protected:
  int32_t verCheckPreChecks();
  int32_t underlyingRead(char* buff, uint32_t size);
  int32_t underlyingWrite(const char* buff, uint32_t size);

private:
    gnutls_session_t session;
  bool handshakecomplete;
};

#endif
