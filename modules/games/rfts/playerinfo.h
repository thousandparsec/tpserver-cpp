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
   static std::string& appAllVictoryPoints(std::string& msg);
   static void clear();

   void setTransportId(uint32_t tid);
   const uint32_t getTransportId() const;

   void addVictoryPoints(uint32_t vp);
   const uint32_t getVictoryPoints() const;
   std::string getVictoryPointsStr() const;

   bool addShipTech(uint32_t points);
   const char getShipTechLevel() const;
   const uint32_t getShipTechPoints() const;

   const bool upgradePdbs() const;
   void clearPdbUpgrade();

 private:
   PlayerInfo(uint32_t playerId);
   PlayerInfo(const PlayerInfo&);
   PlayerInfo& operator=(const PlayerInfo&);
   ~PlayerInfo();

   static std::map<uint32_t,PlayerInfo*> infos;

   uint32_t transportDesignId;
   uint32_t victoryPoints;
   uint32_t shipTech;
   uint32_t globalRP;
   uint32_t playerId;
   bool upgrade;

   static const unsigned TECH_2 = 400;
   static const unsigned TECH_3 = 1000;
   static const unsigned TECH_4 = 2000;
};

}

#endif
