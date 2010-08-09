#ifndef PLAYERAGENT_H
#define PLAYERAGENT_H
/*  PlayerAgent class
 *
 *  Copyright (C) 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/common.h>
#include <tpserver/player.h>
#include <tpserver/playerconnection.h>

class PlayerAgent {
public:
  typedef boost::shared_ptr<PlayerAgent> Ptr;

  PlayerAgent( PlayerConnection::Ptr connection, Player::Ptr nplayer );
  ~PlayerAgent();

  Player::Ptr getPlayer() const;
  
  void processIGFrame( InputFrame::Ptr frame );

private:

  void processPermDisabled( InputFrame::Ptr frame );

  void processGetObjectById( InputFrame::Ptr frame );
  void processGetObjectByPos( InputFrame::Ptr frame );
  void processGetObjectIds( InputFrame::Ptr frame );
  void processGetObjectIdsByPos( InputFrame::Ptr frame );
  void processGetObjectIdsByContainer( InputFrame::Ptr frame );
  
  void processGetObjectDesc( InputFrame::Ptr frame );
  void processGetObjectTypes( InputFrame::Ptr frame );

  void processGetOrderQueue( InputFrame::Ptr frame );
  void processGetOrderQueueIds( InputFrame::Ptr frame );
  
  void processGetOrder( InputFrame::Ptr frame );
  void processAddOrder( InputFrame::Ptr frame );
  void processRemoveOrder( InputFrame::Ptr frame );
  void processDescribeOrder( InputFrame::Ptr frame );
  void processGetOrderTypes( InputFrame::Ptr frame );
  void processProbeOrder( InputFrame::Ptr frame );
  void processGetBoards( InputFrame::Ptr frame );
  void processGetBoardIds( InputFrame::Ptr frame );
  void processGetMessages( InputFrame::Ptr frame );
  void processPostMessage( InputFrame::Ptr frame );
  void processRemoveMessages( InputFrame::Ptr frame );

  void processGetResourceDescription( InputFrame::Ptr frame );
  void processGetResourceTypes( InputFrame::Ptr frame );

  void processGetPlayer( InputFrame::Ptr frame );
  void processGetPlayerIds( InputFrame::Ptr frame );

  void processGetCategory( InputFrame::Ptr frame );
  void processGetCategoryIds( InputFrame::Ptr frame );
  void processGetDesign( InputFrame::Ptr frame );
  void processAddDesign( InputFrame::Ptr frame );
  void processModifyDesign( InputFrame::Ptr frame );
  void processGetDesignIds( InputFrame::Ptr frame );
  void processGetComponent( InputFrame::Ptr frame );
  void processGetComponentIds( InputFrame::Ptr frame );
  void processGetProperty( InputFrame::Ptr frame );
  void processGetPropertyIds( InputFrame::Ptr frame );
  
  void processTurnFinished( InputFrame::Ptr frame );

  /**
   * Checks if version is at least the one passed, if not then throws FrameException
   */
  void versionCheck( InputFrame::Ptr frame, ProtocolVersion min_version );
  /**
   * Checks if the length of the frame is equal to the given value, if not then 
   * throws FrameException.
   */
  void lengthCheck( InputFrame::Ptr frame, uint32_t length );
  /**
   * Checks if the length of the frame is at least the size of the given value, if 
   * not, throws FrameException.
   */
  void lengthCheckMin( InputFrame::Ptr frame, uint32_t length );

  /**
   * Checks for initial length 4 of frame data, then unpacks the number of components.
   * If not enough data 4 + 4 * number then returns 0. If 0 also returns 0. Anyway on
   * 0 throws FrameException.
   *
   * At the end if successful, sends the sequence via connection.
   */
  int queryCheck( InputFrame::Ptr frame );

  PlayerAgent(PlayerAgent & rhs);
  PlayerAgent operator=(PlayerAgent & rhs);

  /// Reference to PlayerConnection
  PlayerConnection::Ref ref_connection;
  /// Temp connection is created when entering processIGFrame, and freed afterwards
  PlayerConnection::Ptr temp_connection;
  Player::Ptr player;

  // blocked default constructor
  PlayerAgent() {}
};

#endif
