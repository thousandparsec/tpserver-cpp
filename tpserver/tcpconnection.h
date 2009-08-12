
#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
/*  Tcp Connection class
 *
 *  Copyright (C) 2009, Kornel Kisielewicz and the Thousand Parsec Project
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

#include <tpserver/connection.h>
#include <tpserver/inputframe.h>
#include <tpserver/outputframe.h>
#include <tpserver/packable.h>
#include <queue>

class TcpConnection: public Connection {
  public:
    virtual ~TcpConnection();

    virtual void close();
    
    // DEPRECATED
    OutputFrame* createFrame(InputFrame* oldframe = NULL);

    // DEPRECATED
    virtual void sendFrame( OutputFrame* frame );

    void process();
    void processWrite();

    ProtocolVersion getProtocolVersion();

    void sendFail(InputFrame* oldframe, FrameErrorCode code, const std::string& error );
    void sendSequence(InputFrame* oldframe, size_t sequence_size );
    void send(InputFrame* oldframe, const Packable* packable );
    void send(InputFrame* oldframe, const Packable::Ptr packable );
    void sendOK(InputFrame* oldframe, const std::string& message );
    void sendModList(InputFrame* oldframe, FrameType ft, uint32_t sequence, const IdModList& modlist,
        uint32_t count, uint32_t start, uint64_t fromtime );
  protected:
    TcpConnection(int fd, Type aType);
  
    virtual int32_t underlyingRead(char* buff, uint32_t size);
    virtual int32_t underlyingWrite(const char* buff, uint32_t size);
  
    void sendString( const std::string& str );

    void processVersionCheck();
    virtual void processNormalFrame() = 0;
    virtual void processLogin() = 0;
    virtual int32_t verCheckPreChecks();
    virtual int32_t verCheckLastChance();
    
    virtual bool readFrame( InputFrame* recvframe );

    bool queueEmpty() const;
    void clearQueue();

    bool getAuth( InputFrame* frame, std::string& username, std::string& password );
    // used by playerhttpconnection
    std::string getHeader() const;

  private:
    /// Blocked to disallow non-fd creation
    TcpConnection() {}

    // TODO: change to std::string
    std::string header_buffer;
    char* rdatabuff;
    uint32_t rbuffused;
  
    std::string send_buffer;
    size_t send_buffer_pos;

    std::queue<OutputFrame*> sendqueue;
  
    bool sendandclose;

  protected:
    ProtocolVersion version;

    bool paddingfilter;
};

#endif
