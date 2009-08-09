/*
 *  AVA Combat
 *
 *  Copyright (C) 2004-2005, 2008  Lee Begg and the Thousand Parsec Project
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

#include "tpserver/object.h"
#include "fleet.h"
#include "planet.h"
#include "tpserver/objecttypemanager.h"
#include "tpserver/message.h"
#include "tpserver/game.h"
#include "tpserver/player.h"
#include "tpserver/playermanager.h"
#include "tpserver/prng.h"
#include "tpserver/designstore.h"
#include "tpserver/design.h"
#include "tpserver/logging.h"
#include "tpserver/property.h"
#include "tpserver/resourcemanager.h"
#include "tpserver/resourcedescription.h"


#include "avacombat.h"

#include <iostream>

AVACombat::AVACombat(){
  c1 = NULL;
  c2 = NULL;
}

AVACombat::~AVACombat(){
}

void AVACombat::setCombatants(IGObject *a, IGObject *b){
  c1 = a;
  c2 = b;
}

bool AVACombat::isAliveCombatant1(){
  return (c1 != NULL);
}

bool AVACombat::isAliveCombatant2(){
  return (c2 != NULL);
}

// This routine handles one combat round.
// fleet1 and fleet2 are the combatants.
// msg1 is sent to the owner of fleet1,
// msg2 is sent to the owner of fleet2.
//
// Assume that 'number of missiles/torpedoes a ship can carry'
// really means 'number of missiles/torpedoes a ship can fire in one round'.
// Ships have an infinite amount of ammunition.
//
// Further assume that everyone fires his/her full load,
// then all the damage is done.  No 'this ship was destroyed
// in this round, so it can't fire any missiles'.
//
// I keep (for now) the policy of always targetting the largest
// ship.
//
// Likelihood of (correct) impact is 100% for missiles, and
// (1-dodge likelihood) for torpedoes.
//
// How about armor as a recurring damage absorber?  Like
// 20HP of damage hitting a ship that has 5HP of armor
// results in only 15HP of damage to that ship.  The next
// round, another 15HP of damage hits the ship, resulting
// in another 10HP of damage.  The armor itself is not
// damaged until the ship is destroyed.
//
// Keep the same damage effects?  Ships are at full
// capacity until they are destroyed?
//
// All that being said, I decreased the damage the two sides
// do to each other by a random 0-39% because I wanted some
// randomness involved.
bool AVACombat::doCombatRound( Fleet*   fleet1,
                               Message* msg1,
                               Fleet*   fleet2,
                               Message* msg2)
{
    Logger::getLogger()->debug("doCombatRound Entering");
    Game* game = Game::getGame();
    DesignStore* ds = game->getDesignStore();
    ResourceManager* resman = game->getResourceManager();
    Random* rand = Game::getGame()->getRandom();
    int r1 = rand->getInRange(0U, 40U);
    int r2 = rand->getInRange(0U, 40U);
    uint32_t damage1 = ( 2 * r1) / 100;
    uint32_t damage2 = ( 2 * r2) / 100;

    typedef std::map<uint32_t, std::pair<uint32_t, uint32_t> > weaponList;
//                   ID/TYPE             AMOUNT
    typedef std::map<uint32_t, uint32_t> shipList;

    std::set<uint32_t> tubes;
    tubes.insert(ds->getPropertyByName("AlphaMissileTube"));
    tubes.insert(ds->getPropertyByName("BetaMissileTube"));
    tubes.insert(ds->getPropertyByName("GammaMissileTube"));
    tubes.insert(ds->getPropertyByName("DeltaMissileTube"));
    tubes.insert(ds->getPropertyByName("EpsilonMissileTube"));
    tubes.insert(ds->getPropertyByName("OmegaTorpedoeTube"));
    tubes.insert(ds->getPropertyByName("UpsilonTorpedoeTube"));
    tubes.insert(ds->getPropertyByName("TauTorpedoeTube"));
    tubes.insert(ds->getPropertyByName("SigmaTorpedoeTube"));
    tubes.insert(ds->getPropertyByName("RhoTorpedoeTube"));
    tubes.insert(ds->getPropertyByName("XiTorpedoeTube"));


    std::map<uint32_t, uint32_t> tubeList;

    weaponList fleet1weaponry, fleet2weaponry;
    shipList fleet1ships, fleet2ships;
    std::map<double, uint32_t> fleet1tubes, fleet2tubes;
    std::map<uint32_t, uint32_t>fleet1usable;
    std::map<uint32_t, uint32_t>fleet2usable;

    fleet1weaponry = fleet1->getResources();
    fleet2weaponry = fleet2->getResources();

    fleet1ships = fleet1->getShips();
    fleet2ships = fleet2->getShips();

    //get usable weapons fleet 1
    for (shipList::iterator itcurr = fleet1ships.begin(); itcurr != fleet1ships.end(); ++itcurr) {
        for (std::set<uint32_t>::iterator tubeit = tubes.begin(); tubeit != tubes.end(); ++tubeit) {
            if (ds->getDesign(itcurr->first)->getPropertyValue(*tubeit) > 0.0) {
                Logger::getLogger()->debug("Found usable Tube, inserting into Fleet 1");
                fleet1tubes[ds->getDesign(itcurr->first)->getPropertyValue(*tubeit)] += 1;
            }
        }

        for (weaponList::iterator weapit = fleet1weaponry.begin(); weapit != fleet1weaponry.end(); ++weapit) {
            Design* weapDesign;
            std::string weapName = resman->getResourceDescription(weapit->first)->getNameSingular();
            std::set<uint32_t>dIDs = ds->getDesignIds();
            for (std::set<uint32_t>::iterator dit = dIDs.begin(); dit != dIDs.end(); ++dit) {
                if (weapName == ds->getDesign(*dit)->getName()) {
                    weapDesign = ds->getDesign(*dit);
                    Logger::getLogger()->debug("Found design %s", weapDesign->getName().c_str());
                }
            }


            for (std::map<double, uint32_t>::iterator tubeit = fleet1tubes.begin(); tubeit != fleet1tubes.end(); ++tubeit) {
                uint32_t propID = ds->getPropertyByName("MissileSize");
                //reading the property value is causing a crash.  propID is set to the correct value
                if (fleet1tubes.find(weapDesign->getPropertyValue(propID)) != fleet1tubes.end()) {
                    if (fleet1->removeResource(weapit->first, 1)) {
                        fleet1usable[weapit->first] += 1;
                    }
                }
            }
        }
    }

    //get usable weapons fleet 2
    for (shipList::iterator itcurr = fleet2ships.begin(); itcurr != fleet2ships.end(); ++itcurr) {
        for (std::set<uint32_t>::iterator tubeit = tubes.begin(); tubeit != tubes.end(); ++tubeit) {
            if (ds->getDesign(itcurr->first)->getPropertyValue(*tubeit) > 0.0) {
                Logger::getLogger()->debug("Found usable Tube, inserting into Fleet 2");
                fleet2tubes[ds->getDesign(itcurr->first)->getPropertyValue(*tubeit)] += 1;
            }
        }

        for (weaponList::iterator weapit = fleet2weaponry.begin(); weapit != fleet2weaponry.end(); ++weapit) {
            Design* weapDesign;
            std::string weapName = resman->getResourceDescription(weapit->first)->getNameSingular();
            std::set<uint32_t>dIDs = ds->getDesignIds();
            for (std::set<uint32_t>::iterator dit = dIDs.begin(); dit != dIDs.end(); ++dit) {
                if (weapName == ds->getDesign(*dit)->getName()) {
                    weapDesign = ds->getDesign(*dit);
                    Logger::getLogger()->debug("Found design %s", weapDesign->getName().c_str());
                }
            }
            for (std::map<double, uint32_t>::iterator tubeit = fleet2tubes.begin(); tubeit != fleet2tubes.end(); ++tubeit) {
                if (fleet2tubes.find(weapDesign->getPropertyValue(ds->getPropertyByName("MissileSize"))) != fleet2tubes.end()) {
                    if (fleet2->removeResource(weapit->first, 1)) {
                        fleet2usable[weapit->first] += 1;
                    }
                }
            }
        }
    }

for (std::map<uint32_t, uint32_t>::iterator it = fleet1usable.begin(); it != fleet1usable.end(); ++it){
std::cout << "USABLE MISSILE NUMBER: " <<it->first << "\n\n\n";
}



    bool tte = false;

    std::string body1, body2;

    if ( ! false) {
        body1 += "Your fleet was destroyed. ";
        body2 += "You destroyed their fleet. ";
        c1 = NULL;
        tte = true;
    };
    if ( ! false) {
        body2 += "Your fleet was destroyed.";
        body1 += "You destroyed their fleet.";
        c2 = NULL;
        tte = true;
    }
    if ( tte) {
        msg1->setBody( body1);
        msg2->setBody( body2);
    }

    Logger::getLogger()->debug("doCombatRound Exiting");
    return false;
}


// Combat always takes place between two fleets.  If one combatant
// or the other is actually a planet, the planet simulated by two
// battleships.
void AVACombat::doCombat()
{
    Fleet * f1, *f2;

    uint32_t obT_Fleet = Game::getGame()->getObjectTypeManager()->getObjectTypeByName("Fleet");
    uint32_t obT_Planet = Game::getGame()->getObjectTypeManager()->getObjectTypeByName("Planet");

    
    // If one combatant or the other is actually a planet,
    // simulate this with two battleships.
    if ( c1->getType() == obT_Fleet) {
        f1 = ( Fleet*) ( c1->getObjectBehaviour());
    }
    else if ( c1->getType() == obT_Planet) {
        f1 = new Fleet();
        f1->addShips( 3, 2);
        f1->setOwner( ( ( Planet*) c1->getObjectBehaviour())->getOwner());
    }
    if ( c2->getType() == obT_Fleet) {
        f2 = ( Fleet*) ( c2->getObjectBehaviour());
    }
    else if ( c2->getType() == obT_Planet) {
        f2 = new Fleet();
        f2->addShips( 3, 2);
        f2->setOwner( ( ( Planet*) c2->getObjectBehaviour())->getOwner());
    }
    if ( f1 == NULL || f2 == NULL) {
        return;
    }

    Message *msg1, *msg2;
    msg1 = new Message();
    msg2 = new Message();
    msg1->setSubject( "Combat");
    msg2->setSubject( "Combat");
    msg1->addReference( rst_Object, c1->getID());
    msg1->addReference( rst_Object, c2->getID());
    msg1->addReference( rst_Player, f2->getOwner());
    msg2->addReference( rst_Object, c2->getID());
    msg2->addReference( rst_Object, c1->getID());
    msg2->addReference( rst_Player, f1->getOwner());

    while ( doCombatRound( f1, msg1, f2, msg2)) {
        ;
    }

    Game::getGame()->getPlayerManager()->getPlayer( f1->getOwner())->postToBoard( msg1);
    Game::getGame()->getPlayerManager()->getPlayer( f2->getOwner())->postToBoard( msg2);

    return;
}

