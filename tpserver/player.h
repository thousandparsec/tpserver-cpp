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
#include <map>
#include <string>
#include <stdint.h>

class Frame;
class Message;
class PlayerView;

class Player {
public:
  Player();
  ~Player();

  void setName(const std::string& newname);
  void setPass(const std::string& newpass);
  void setEmail(const std::string& newemail);
  void setComment(const std::string& newcomm);
  void setId(uint32_t newid);
  void setIsAlive(bool na);
  void setScore(uint32_t key, uint32_t value);
  void setBoardId(uint32_t nbi);
  

  std::string getName() const;
  std::string getPass() const;
  std::string getEmail() const;
  std::string getComment() const;
  uint32_t getID() const;
  bool isAlive() const;
  uint32_t getScore(uint32_t key) const;
  std::map<uint32_t, uint32_t> getAllScores() const;
  uint32_t getBoardId() const;
  uint64_t getModTime() const;

  void postToBoard(Message* msg);
  
  PlayerView* getPlayerView() const;
  
  void packFrame(Frame* frame);
  

  //for persistence only
  void setModTime(uint64_t nmt);

private:

  std::string name;
  std::string passwd;
  std::string email;
  std::string comment;

  uint32_t pid;
  uint32_t boardid;
  uint64_t modtime;
  
  PlayerView* playerview;
  
  bool alive;
  std::map<uint32_t, uint32_t> score;
  
  void touchModTime();

  Player(Player & rhs);

  Player operator=(Player & rhs);

};

#endif
