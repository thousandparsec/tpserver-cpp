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
#include <sstream>

#include <tpserver/game.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/designstore.h>

#include "rfts.h"

#include "playerinfo.h"

namespace RFTS_ {

using std::map;
using std::string;
using std::set;

map<uint32_t,PlayerInfo*> PlayerInfo::infos;

PlayerInfo& PlayerInfo::getPlayerInfo(uint32_t pid) {
   
   if(infos.find(pid) == infos.end())
      infos[pid] = new PlayerInfo(pid);

   return *infos[pid];
}

string& PlayerInfo::appAllVictoryPoints(string& msg) {
   msg += "<br><br> Current Victory Points:<br>";
   set<uint32_t> players = Game::getGame()->getPlayerManager()->getAllIds();
   for(set<uint32_t>::iterator i = players.begin(); i != players.end(); ++i)
      msg += infos[*i]->getVictoryPointsStr() + "<br>";
   return msg;
}

void PlayerInfo::clear() {
   for(map<uint32_t,PlayerInfo*>::iterator i = infos.begin(); i != infos.end(); ++i)
      delete i->second;
}

PlayerInfo::PlayerInfo(uint32_t pid)
   : transportDesignId(0), victoryPoints(0), shipTech(0), globalRP(0),
       playerId(pid), upgrade(false) {

}

void PlayerInfo::setTransportId(uint32_t tid) {
   transportDesignId = tid;
}

const uint32_t PlayerInfo::getTransportId() const {
   return transportDesignId;
}

const bool PlayerInfo::upgradePdbs() const {
   return upgrade;
}

void PlayerInfo::clearPdbUpgrade() {
   upgrade = false;
}

void PlayerInfo::addVictoryPoints(uint32_t vp) {
   victoryPoints += vp;
}

const uint32_t PlayerInfo::getVictoryPoints() const {
   return victoryPoints;
}

string PlayerInfo::getVictoryPointsStr() const {
   std::ostringstream vp;
   vp << victoryPoints;
   return Game::getGame()->getPlayerManager()->getPlayer(playerId)->getName() +
          string(" victory points: ") + vp.str();
}

bool PlayerInfo::addShipTech(uint32_t points) {
   shipTech += points;

   bool justUpgraded = false;

   Game* game = Game::getGame();
   Player *player = game->getPlayerManager()->getPlayer(playerId);

   if(shipTech >= TECH_2 && shipTech - points < TECH_2)
   {
      game->getDesignStore()->designCountsUpdated(createMarkDesign(player, '2'));
      justUpgraded = true;
   }

   if(shipTech >= TECH_3 && shipTech - points < TECH_3)
   {
      game->getDesignStore()->designCountsUpdated(createMarkDesign(player, '3'));
      justUpgraded = true;
   }
   
   if(shipTech >= TECH_4 && shipTech - points < TECH_4)
   {
      game->getDesignStore()->designCountsUpdated(createMarkDesign(player, '4'));
      justUpgraded = true;
   }

    upgrade |= justUpgraded;

   return justUpgraded;
}

const char PlayerInfo::getShipTechLevel() const {
   if(shipTech >= TECH_4)
      return '4';
   if(shipTech >= TECH_3)
      return '3';
   if(shipTech >= TECH_2)
      return '2';
   return '1';
}

const uint32_t PlayerInfo::getShipTechPoints() const {
   return shipTech;
}


}
