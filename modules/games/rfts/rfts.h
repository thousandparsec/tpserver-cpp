#ifndef RFTS_H
#define RFTS_H
/*  RFTS rulesset class
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
#include <vector>

#include <tpserver/ruleset.h>

class IGObject;
class Component;
class Design;
class Player;
class ResourceDescription;

namespace RFTS_ {

class ProductionInfo;

class Rfts : public ::Ruleset {
 public:
   Rfts();
   virtual ~Rfts();

   virtual std::string getName();
   virtual std::string getVersion();
      
   virtual void initGame();
   virtual void createGame();
   virtual void startGame();

   virtual bool onAddPlayer(Player *player);
   virtual void onPlayerAdded(Player *player);

   static const ProductionInfo& getProductionInfo();

   // used for calculating cone-like universe (for wrapping map)
   static const uint64_t UNIVERSE_TOTAL_SCALE = 1000000000ll;
   static const uint32_t UNIVERSE_WIDTH = 4;
   static const uint32_t UNIVERSE_HEIGHT = 3;
   static const uint32_t UNIVERSE_DEPTH = UNIVERSE_WIDTH * UNIVERSE_HEIGHT;
   
 private:    
   
   void setObjectTypes() const;
   void setOrderTypes() const;

   void createProperties();
   void createComponents();
   void createResources();
   std::pair<ResourceDescription*,ResourceDescription*> createPdbResource(char level) const;

   void createUniverse();
   IGObject* createStarSystem(IGObject& universe, const std::string& name,
                           double unitX, double unitY);
   IGObject* createPlanet(IGObject& parentStarSys, const std::string& name,
                            const Vector3d& location);

   IGObject* choosePlayerPlanet() const;

   static const uint32_t MAX_PLAYERS = 4; // to be data-driven?

};

const Vector3d calcUniverseCoord(double unitX, double unitY);

Component* createEngineComponent(char techLevel);
Component* createBattleComponent(char techLevel);
Component* createTransportComponent();

Design* createMarkDesign(Player *owner, char level);
Design* createScoutDesign(Player *owner);
Design* createTransportDesign(Player *ownder);

}

#endif
