#ifndef PLAYER_H
#define PLAYER_H
/*  Player class
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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
class Board;
class Message;

class Player {
      public:
	Player();
	~Player();

	void setName(const std::string& newname);
	void setPass(const std::string& newpass);
	void setConnection(PlayerConnection * newcon);
        void setId(uint32_t newid);

	void setVisibleObjects(std::set<unsigned int> vis);
	bool isVisibleObject(unsigned int objid);
        std::set<uint32_t> getVisibleObjects() const;

	void addVisibleDesign(unsigned int designid);
	void addUsableDesign(unsigned int designid);
	void removeUsableDesign(unsigned int designid);
	bool isUsableDesign(unsigned int designid) const;
	std::set<unsigned int> getUsableDesigns() const;
        std::set<uint32_t> getVisibleDesigns() const;

	void addVisibleComponent(unsigned int compid);
	void addUsableComponent(unsigned int compid);
	void removeUsableComponent(unsigned int compid);
	bool isUsableComponent(unsigned int compid);
        std::set<uint32_t> getVisibleComponents() const;
        std::set<uint32_t> getUsableComponents() const;

	void postToBoard(Message* msg);

	std::string getName() const;
	std::string getPass() const;
	PlayerConnection *getConnection();
	int getID();
        uint32_t getBoardId() const;
        void setBoardId(uint32_t nbi);

	void packFrame(Frame* frame);

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

	PlayerConnection *curConnection;
	std::string name;
	std::string passwd;
	uint32_t pid;
    uint32_t boardid;

	 Player(Player & rhs);

	Player operator=(Player & rhs);

	std::set<unsigned int> visibleObjects;
	unsigned int currObjSeq;

	std::set<unsigned int> visibleDesigns;
	std::set<unsigned int> usableDesigns;

	std::set<unsigned int> visibleComponents;
	std::set<unsigned int> usableComponents;

};

#endif
