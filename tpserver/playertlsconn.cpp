/*  Player TLS Connection object, supports ipv4 and ipv6
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

#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <gnutls/gnutls.h>

#include "logging.h"
#include "net.h"
#include "frame.h"
#include "game.h"
#include "player.h"
#include "tlsmanager.h"

#include "playertlsconn.h"


PlayerTlsConnection::PlayerTlsConnection(int fd) : PlayerConnection(fd), handshakecomplete(false){
    TlsManager::getInstance()->reference();

    gnutls_init (&session, GNUTLS_SERVER);
    /* Use the default priorities, plus, export cipher suites.
    */
    gnutls_set_default_export_priority (session);

    gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, TlsManager::getInstance()->getCredentials());

    /* request client certificate if any.
    */
    //gnutls_certificate_server_set_request (session, GNUTLS_CERT_REQUEST);

    gnutls_dh_set_prime_bits (session, 1024);

    gnutls_transport_set_ptr (session, (gnutls_transport_ptr_t) fd);
}

PlayerTlsConnection::~PlayerTlsConnection(){
    if (status != DISCONNECTED) {
            close();
    }
    TlsManager::getInstance()->dereference();
}


void PlayerTlsConnection::close(){
  if(sendqueue.empty()){
    gnutls_bye (session, GNUTLS_SHUT_WR);
    gnutls_deinit (session);
  }
  PlayerConnection::close();
}





int32_t PlayerTlsConnection::verCheckPreChecks(){
  if(handshakecomplete){
    return 1;
  }else{
    int32_t ret = gnutls_handshake (session);
    if (ret == 0){
      handshakecomplete = true;
      return 1;
    }else if(gnutls_error_is_fatal(ret)){
        Logger::getLogger()->error("PlayerTlsConnection: could not handshake - %s", gnutls_strerror (ret));
        return -1;
    }
    //still waiting
    return -2;
  }
}


int32_t PlayerTlsConnection::underlyingRead(char* buff, uint32_t size){
  int32_t len = gnutls_record_recv(session, buff, size);
  if(len < 0){
    if(len == GNUTLS_E_INTERRUPTED || len == GNUTLS_E_AGAIN){
      len = -2;
    }else{
      Logger::getLogger()->debug("tls read error - %s", gnutls_strerror(len));
      len = -1;
    }
  }
  return len;
}

int32_t PlayerTlsConnection::underlyingWrite(const char* buff, uint32_t size){
  int32_t len = gnutls_record_send(session, buff, size);
  if(len < 0){
    if(len == GNUTLS_E_INTERRUPTED || len == GNUTLS_E_AGAIN){
      len = -2;
    }else{
      Logger::getLogger()->debug("tls write error - %s", gnutls_strerror(len));
      len = -1;
    }
  }
  return len;
}
