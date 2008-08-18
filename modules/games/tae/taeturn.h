#ifndef TAETURN_H
#define TAETURN_H
/*  TaeTurn class, the end of turn process for tae
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
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

#include <queue>
#include <tpserver/turnprocess.h>
#include "fleetbuilder.h"
#include "fleet.h"

class TaeTurn : public TurnProcess{
  public:
    TaeTurn(FleetBuilder* fb);
    virtual ~TaeTurn();
    
    virtual void doTurn();
  
    void setFleetType(uint32_t ft);
    void setPlanetType(uint32_t pt);
    void setGameOver(bool isOver);
    
    //Queue up a conflict to be resolved at the next possible turn
    void queueCombatTurn(bool internal, std::map<uint32_t, uint32_t> com);
    //Adds one reinforcement to the player
    void addReinforcement(uint32_t player); 
 
    std::set<uint32_t> getContainerIds() const;
    
  private:
    FleetBuilder* fleetBuilder;
    int playerTurn;
    uint32_t planettype;
    uint32_t fleettype;
    std::set<uint32_t> containerids;
    bool combat;
    bool isInternal;
    bool isGameOver;
    std::map<uint32_t, uint32_t> combatants;
    std::map<uint32_t, int> strength;
    std::queue<std::pair<bool, std::map<uint32_t, uint32_t> > > combatQueue;

    void awardArtifacts();
    void initCombat();
    void doCombatTurn();
    void sendHome(uint32_t fleet);
    void rebuildRegion(uint32_t system);
    void gameOver();
};

#endif
