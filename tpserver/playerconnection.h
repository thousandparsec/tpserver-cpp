#ifndef PLAYERCONNECTION_H
#define PLAYERCONNECTION_H
/*  Player Connection class
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

class PlayerAgent;
class PlayerConnection: public TcpConnection {
  public:
    /// Shared pointer typedef
    typedef boost::shared_ptr< PlayerConnection > Ptr;
    
    /// Weak pointer typedef
    typedef boost::weak_ptr< PlayerConnection > Ref;

    virtual ~PlayerConnection();

    PlayerConnection(int fd);

  protected:
    void processGetFeaturesFrame(InputFrame::Ptr frame);

  private:
    virtual void processNormalFrame();
    virtual void processLogin();

    void processGetGameInfoFrame(InputFrame::Ptr frame);
    void processSetFilters(InputFrame::Ptr frame);
    void processTimeRemainingFrame(InputFrame::Ptr frame);
    void processLoginFrame(InputFrame::Ptr frame);
    void processAccountFrame(InputFrame::Ptr frame);
    void processPingFrame(InputFrame::Ptr frame);
    
    boost::shared_ptr<PlayerAgent> playeragent;
    uint64_t lastpingtime;
};

#endif
