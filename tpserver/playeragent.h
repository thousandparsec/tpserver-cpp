#ifndef PLAYERAGENT_H
#define PLAYERAGENT_H
/*  PlayerAgent class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

#include <set>
#include <string>

class PlayerConnection;
class Frame;
class Player;

class PlayerAgent {
public:
  PlayerAgent();
  ~PlayerAgent();

  void setConnection(PlayerConnection * newcon);
  PlayerConnection *getConnection() const;
  
  void setPlayer(Player* newplayer);
  Player* getPlayer() const;
  
  void processIGFrame(Frame * frame);

private:

  void processPermDisabled(Frame * frame);

  void processGetObjectById(Frame * frame);
  void processGetObjectByPos(Frame * frame);
  void processGetObjectIds(Frame * frame);
  void processGetObjectIdsByPos(Frame * frame);
  void processGetObjectIdsByContainer(Frame * frame);
  void processGetOrder(Frame * frame);
  void processAddOrder(Frame * frame);
  void processRemoveOrder(Frame * frame);
  void processDescribeOrder(Frame * frame);
  void processGetOrderTypes(Frame * frame);
  void processProbeOrder(Frame * frame);
  void processGetBoards(Frame * frame);
  void processGetBoardIds(Frame * frame);
  void processGetMessages(Frame * frame);
  void processPostMessage(Frame * frame);
  void processRemoveMessages(Frame * frame);

  void processGetResourceDescription(Frame * frame);
  void processGetResourceTypes(Frame* frame);

  void processGetPlayer(Frame* frame);

  void processGetCategory(Frame* frame);
  void processGetCategoryIds(Frame* frame);
  void processGetDesign(Frame* frame);
  void processAddDesign(Frame* frame);
  void processModifyDesign(Frame* frame);
  void processGetDesignIds(Frame* frame);
  void processGetComponent(Frame* frame);
  void processGetComponentIds(Frame* frame);
  void processGetProperty(Frame* frame);
  void processGetPropertyIds(Frame* frame);

  PlayerAgent(PlayerAgent & rhs);
  PlayerAgent operator=(PlayerAgent & rhs);

  PlayerConnection *curConnection;
  Player* player;

};

#endif
