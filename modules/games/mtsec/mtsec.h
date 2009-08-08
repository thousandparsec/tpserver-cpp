#ifndef MTSEC_H
#define MTSEC_H
/*  MTSec rulesset class
 *
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/ruleset.h>
#include <tpserver/designstore.h>
#include <tpserver/player.h>

namespace MTSecRuleset {

class MTSec : public Ruleset{
 public:
  MTSec();
  virtual ~MTSec();

  std::string getName();
  std::string getVersion();
  void initGame();
  void createGame();
  void startGame();
  bool onAddPlayer(Player::Ptr player);
  void onPlayerAdded(Player::Ptr player);

 protected:
  // Design definition methods
  Design::Ptr createScoutDesign( Player::Ptr owner);
  Design::Ptr createBattleScoutDesign( Player::Ptr owner);

  // Object creation methods
  IGObject::Ptr createSiriusSystem( IGObject::Ptr mw_galaxy);
  IGObject::Ptr createStarSystem( IGObject::Ptr mw_galaxy);
  IGObject::Ptr createSolSystem( IGObject::Ptr mw_galaxy);
  IGObject::Ptr createAlphaCentauriSystem( IGObject::Ptr mw_galaxy);

  void createProperties();
  void createComponents();
  void createTechTree();
  
  void createResources();

  IGObject::Ptr createEmptyFleet( Player::Ptr     owner,
                              IGObject::Ptr   star,
                              std::string fleetName);
  void makeNewPlayerFleet( Player::Ptr player, IGObject::Ptr star);
  IGObject::Ptr makePlayerHomePlanet( Player::Ptr player, IGObject::Ptr star);
  IGObject::Ptr makeNewPlayerStarSystem( Player::Ptr player);
  void setNewPlayerTech( Player::Ptr player);
  Design::Ptr createAlphaMissileDesign( Player::Ptr owner);

 private:
  xmlImport *xmlImporter;

};

}//end namespace

#endif

