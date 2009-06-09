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

#include "tpserver/ruleset.h"

class MTSec : public Ruleset{
 public:
  MTSec();
  virtual ~MTSec();

  std::string getName();
  std::string getVersion();
  void initGame();
  void createGame();
  void startGame();
  bool onAddPlayer(Player* player);
  void onPlayerAdded(Player* player);

 protected:
  // Property definition methods
  void createSpeedProp();
  void createAmmoCostProp();
  void createAmmoExplosivenessProp();
  void createAmmoSizeProp();
  void createFirepowerProp();
  void createMissileCostProp();
  void createMissileFirepowerProp();
  void createMissileSizeProp();
  void createHitPointsProp();
  void createHPProp();
  void createBuildTimeProp();
  void createArmorProp();
  void createColoniseProp();
  void createNumAmmoProp();
  void createNumBayTypesProp();
  void createNumHullsProp();


  // Design definition methods
  Design* createScoutDesign( Player* owner);
  Design* createBattleScoutDesign( Player* owner);


  // Object creation methods
  IGObject* createSiriusSystem( IGObject* mw_galaxy);
  IGObject* createStarSystem( IGObject* mw_galaxy);
  IGObject* createSolSystem( IGObject *mw_galaxy);
  IGObject* createAlphaCentauriSystem( IGObject* mw_galaxy);

  void createProperties();
  void createComponents();
  void createTechTree();
  
  void createResources();

  IGObject* createEmptyFleet( Player*     owner,
                              IGObject*   star,
                              std::string fleetName);
  void makeNewPlayerFleet( Player* player, IGObject* star);
  IGObject* makePlayerHomePlanet( Player* player, IGObject* star);
  IGObject* makeNewPlayerStarSystem( Player* player);
  void setNewPlayerTech( Player* player);

 private:
  std::map<std::string,uint32_t>  propertyIndex;
  uint32_t compMax;
  std::string importFile;

};

#endif
