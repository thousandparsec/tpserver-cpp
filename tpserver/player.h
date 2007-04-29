#ifndef PLAYER_H
#define PLAYER_H
/*  Player class
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include <stdint.h>

class Frame;
class Message;

class Player {
public:
  Player();
  ~Player();

  void setName(const std::string& newname);
  void setPass(const std::string& newpass);
  void setEmail(const std::string& newemail);
  void setComment(const std::string& newcomm);
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
  std::string getEmail() const;
  std::string getComment() const;
  uint32_t getID();
  uint32_t getBoardId() const;
  void setBoardId(uint32_t nbi);

  uint32_t getObjectSequenceKey() const;

  void packFrame(Frame* frame);


private:

  std::string name;
  std::string passwd;
  std::string email;
  std::string comment;

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
