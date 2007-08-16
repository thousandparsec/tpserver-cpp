#ifndef playerinfo_H
#define playerinfo_H
/*  playerinfo class
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
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

namespace RFTS_ {

class PlayerInfo {
 public:
   static PlayerInfo& getPlayerInfo(uint32_t pid);

   void setTransportId(uint32_t tid);
   const uint32_t getTransportId() const;

   void addVictoryPoints(uint32_t vp);
   const uint32_t getVictoryPoints() const;

   void addShipTechPoints(uint32_t points);
   const char getShipTechLevel() const;

 private:
   PlayerInfo();
   PlayerInfo(const PlayerInfo&);
   PlayerInfo& operator=(const PlayerInfo&);
   ~PlayerInfo();

   uint32_t transportDesignId;
   uint32_t victoryPoints;
   uint32_t shipTech;
   uint32_t globalRP;
};


}

#endif
