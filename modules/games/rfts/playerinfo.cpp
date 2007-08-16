/*  playerinfo
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

#include <map>

#include "playerinfo.h"

namespace RFTS_ {

using std::map;

PlayerInfo& PlayerInfo::getPlayerInfo(uint32_t pid) {
   static map<uint32_t,PlayerInfo*> infos;

   if(infos.find(pid) == infos.end())
      infos[pid] = new PlayerInfo();

   return *infos[pid];
}

PlayerInfo::PlayerInfo() : transportDesignId(0), victoryPoints(0) {

}

void PlayerInfo::setTransportId(uint32_t tid) {
   transportDesignId = tid;
}

const uint32_t PlayerInfo::getTransportId() const {
   return transportDesignId;
}

void PlayerInfo::addVictoryPoints(uint32_t vp) {
   victoryPoints += vp;
}

const uint32_t PlayerInfo::getVictoryPoints() const {
   return victoryPoints;
}

}
