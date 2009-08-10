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

class PlayerConnection;
class Frame;

class PlayerAgent {
public:
  PlayerAgent( PlayerConnection* connection, Player::Ptr nplayer );
  ~PlayerAgent();

  PlayerConnection *getConnection() const;
  
  Player::Ptr getPlayer() const;
  
  void processIGFrame( InputFrame* frame );

private:

  void processPermDisabled( InputFrame* frame );

  void processGetObjectById( InputFrame* frame );
  void processGetObjectByPos( InputFrame* frame );
  void processGetObjectIds( InputFrame* frame );
  void processGetObjectIdsByPos( InputFrame* frame );
  void processGetObjectIdsByContainer( InputFrame* frame );
  
  void processGetObjectDesc( InputFrame* frame );
  void processGetObjectTypes( InputFrame* frame );
  
  void processGetOrder( InputFrame* frame );
  void processAddOrder( InputFrame* frame );
  void processRemoveOrder( InputFrame* frame );
  void processDescribeOrder( InputFrame* frame );
  void processGetOrderTypes( InputFrame* frame );
  void processProbeOrder( InputFrame* frame );
  void processGetBoards( InputFrame* frame );
  void processGetBoardIds( InputFrame* frame );
  void processGetMessages( InputFrame* frame );
  void processPostMessage( InputFrame* frame );
  void processRemoveMessages( InputFrame* frame );

  void processGetResourceDescription( InputFrame* frame );
  void processGetResourceTypes( InputFrame* frame );

  void processGetPlayer( InputFrame* frame );
  void processGetPlayerIds( InputFrame* frame );

  void processGetCategory( InputFrame* frame );
  void processGetCategoryIds( InputFrame* frame );
  void processGetDesign( InputFrame* frame );
  void processAddDesign( InputFrame* frame );
  void processModifyDesign( InputFrame* frame );
  void processGetDesignIds( InputFrame* frame );
  void processGetComponent( InputFrame* frame );
  void processGetComponentIds( InputFrame* frame );
  void processGetProperty( InputFrame* frame );
  void processGetPropertyIds( InputFrame* frame );
  
  void processTurnFinished( InputFrame* frame );

  /**
   * Checks if version is at least the one passed, if not then throws FrameException
   */
  void versionCheck( InputFrame* frame, ProtocolVersion min_version );
  /**
   * Checks if the length of the frame is equal to the given value, if not then 
   * throws FrameException.
   */
  void lengthCheck( InputFrame* frame, uint32_t length );
  /**
   * Checks if the length of the frame is at least the size of the given value, if 
   * not, throws FrameException.
   */
  void lengthCheckMin( InputFrame* frame, uint32_t length );

  /**
   * Checks for initial length 4 of frame data, then unpacks the number of components.
   * If not enough data 4 + 4 * number then returns 0. If 0 also returns 0. Anyway on
   * 0 throws FrameException.
   *
   * At the end if successful, sends the sequence via connection.
   */
  int queryCheck( InputFrame* frame );

  PlayerAgent(PlayerAgent & rhs);
  PlayerAgent operator=(PlayerAgent & rhs);

  PlayerConnection *curConnection;
  Player::Ptr player;

  // blocked default constructor
  PlayerAgent() {}
};

#endif
