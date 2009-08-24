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
#include "dummyfleet.h"
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

#include <cmath>

namespace MTSecRuleset {

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
// fleets[2] are the combatants.
// msgs[0] is sent to the owner of fleet1,
// msgs[1] is sent to the owner of fleet2.
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
bool AVACombat::doCombatRound( Fleet*   fleets[],
                               Message* msgs[])
{
    Logger::getLogger()->debug("doCombatRound Entering");
    Game* game = Game::getGame();
    DesignStore* ds = game->getDesignStore();
    ResourceManager* resman = game->getResourceManager();

    typedef std::map<uint32_t, std::pair<uint32_t, uint32_t> > weaponList;
//                   ID/TYPE             AMOUNT
    typedef std::map<uint32_t, uint32_t> shipList;

    std::map<uint32_t, std::string> tubes;
    tubes[ds->getPropertyByName("AlphaMissileTube")] = "AlphaMissileTube";
    tubes[ds->getPropertyByName("BetaMissileTube")] = "BetaMissileTube";
    tubes[ds->getPropertyByName("GammaMissileTube")] = "GammaMissileTube";
    tubes[ds->getPropertyByName("DeltaMissileTube")] = "DeltaMissileTube";
    tubes[ds->getPropertyByName("EpsilonMissileTube")] = "EpsilonMissileTube";
    tubes[ds->getPropertyByName("OmegaTorpedoeTube")] = "OmegaTorpedoeTube";
    tubes[ds->getPropertyByName("UpsilonTorpedoeTube")] = "UpsilonTorpedoeTube";
    tubes[ds->getPropertyByName("TauTorpedoeTube")] = "TauTorpedoeTube";
    tubes[ds->getPropertyByName("SigmaTorpedoeTube")] = "SigmaTorpedoeTube";
    tubes[ds->getPropertyByName("RhoTorpedoeTube")] = "RhoTorpedoeTube";
    tubes[ds->getPropertyByName("XiTorpedoeTube")] = "XiTorpedoeTube";


    std::map<uint32_t, uint32_t> tubeList;
    std::set<Design*> designList;

    weaponList fleetweaponry[2];
    shipList fleetships[2];
    std::map<uint32_t, std::pair<std::string, uint32_t> > fleettubes[2];
    std::map<uint32_t, uint32_t> fleetusable[2];
    //     resourceid, designid

    fleetweaponry[0] = fleets[0]->getResources();
    fleetweaponry[1] = fleets[1]->getResources();

    fleetships[0] = fleets[0]->getShips();
    fleetships[1] = fleets[1]->getShips();

    uint32_t damage[2] = {0};

    for (int i=0; i < 2; i++) {
        for (shipList::iterator itcurr = fleetships[i].begin(); itcurr != fleetships[i].end(); ++itcurr) {
            for (std::map<uint32_t, std::string>::iterator tubeit = tubes.begin(); tubeit != tubes.end(); ++tubeit) {
                if (ds->getDesign(itcurr->first)->getPropertyValue(tubeit->first) > 0.0) {
                    Logger::getLogger()->debug("Found usable Tube %s, inserting into Fleet %d", tubeit->second.c_str(), i);
                    //size of weapon
                    uint32_t weapSizePropID = ds->getPropertyByName(tubeit->second);
                    uint32_t weapNumPropID = ds->getPropertyByName("num-" + tubeit->second);
                    uint32_t weapSizePropValue = static_cast<uint32_t>(ds->getDesign(itcurr->first)->getPropertyValue(weapSizePropID));
                    uint32_t weapNumPropValue = static_cast<uint32_t>(ds->getDesign(itcurr->first)->getPropertyValue(weapNumPropID));
                    if (fleettubes[i].find(weapSizePropValue) != fleettubes[i].end()) {
                        std::pair<std::string, uint32_t>fleetpair = fleettubes[i][weapSizePropValue];
                        fleetpair.second += weapNumPropValue;
                        fleettubes[i][weapSizePropValue] = fleetpair;
                        Logger::getLogger()->debug("Adding %d into Tube List", weapSizePropValue);
                    } else {
                        fleettubes[i][weapSizePropValue] = std::pair<std::string, uint32_t> (tubeit->second, weapNumPropValue);
                    }
                }
            }
    
            for (weaponList::iterator weapit = fleetweaponry[i].begin(); weapit != fleetweaponry[i].end(); ++weapit) {
                Design* weapDesign;
                std::string weapName = resman->getResourceDescription(weapit->first)->getNameSingular();
                std::set<uint32_t>dIDs = ds->getDesignIds();
                for (std::set<uint32_t>::iterator dit = dIDs.begin(); dit != dIDs.end(); ++dit) {
                    if (weapName == ds->getDesign(*dit)->getName()) {
                        weapDesign = ds->getDesign(*dit);
                        Logger::getLogger()->debug("Found design %s", weapDesign->getName().c_str());
    
                        for (std::map<uint32_t, std::pair<std::string, uint32_t> >::iterator tubeit = fleettubes[i].begin(); tubeit != fleettubes[i].end(); ++tubeit) {
                            //property ID, name of the TUBE
                            uint32_t propID = ds->getPropertyByName("MissileSize");
                            //Logger::getLogger()->debug("Checking fleettubes list for propID %s", ds->getProperty(propID)->getName().c_str());
                            uint32_t weapPropVal = static_cast<uint32_t>(weapDesign->getPropertyValue(propID));
                            if (fleettubes[i].find(weapPropVal) != fleettubes[i].end()) {
                                Logger::getLogger()->debug("Found it, trying to remove resource %d from fleet", weapit->first);
                                fleetusable[i][weapit->first] = weapDesign->getDesignId();
                                if (fleets[i]->removeResource(weapit->first, 1)) {
                                    uint32_t explPropID = ds->getPropertyByName("AmmoExplosiveness");
                                    //damage to be deliveredt
                                    damage[!i] += static_cast<uint32_t>(weapDesign->getPropertyValue(explPropID));
                                    Logger::getLogger()->debug("Adding Damage (%d) Total (%d)", static_cast<uint32_t>(weapDesign->getPropertyValue(explPropID)), damage[!i]);
                                } else {
                                    Logger::getLogger()->debug("No available Weapon for this tube!");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    fleets[0]->setDamage(damage[0]);
    fleets[1]->setDamage(damage[1]);

    resolveCombat(fleets[0]);
    resolveCombat(fleets[1]);

    //Tube format: std::map<double, std::pair<std::string, uint32_t> >
    //                   tube size            name         number

    bool tte = false;

    std::string body[2];

    if (fleets[0]->totalShips() == 0) {
        body[0] += "Your fleet was destroyed. ";
        body[1] += "You destroyed their fleet. ";
        c1 = NULL;
        tte = true;
    };
    if (fleets[1]->totalShips() == 0) {
        body[0] += "Your fleet was destroyed.";
        body[1] += "You destroyed their fleet.";
        c2 = NULL;
        tte = true;
    }
    if ( tte) {
        msgs[0]->setBody( body[0]);
        msgs[1]->setBody( body[1]);
    }

    Logger::getLogger()->debug("doCombatRound Exiting");
    return false;
}

void AVACombat::resolveCombat(Fleet* fleet){
    Logger::getLogger()->debug("AVACombat::resolveCombat : Entering");

    Game* game = Game::getGame();
    DesignStore* ds = game->getDesignStore();
    typedef std::map<uint32_t, uint32_t> shipList;
    shipList ships = fleet->getShips();
    uint32_t damage = fleet->getDamage();
    uint32_t hpPropID = ds->getPropertyByName("HitPoints");

    for (shipList::reverse_iterator itcurr = ships.rbegin(); itcurr != ships.rend(); ++itcurr){
        uint32_t shipNumber = itcurr->first;
        uint32_t shipQuantity = itcurr->second;
        Logger::getLogger()->debug("Working on ship %s", ds->getDesign(shipNumber)->getName().c_str());

        uint32_t damageNeeded = static_cast<uint32_t>(ds->getDesign(shipNumber)->getPropertyValue(hpPropID));
        Logger::getLogger()->debug("Damage needed to destroy this ship: %d Damage available: %d", damageNeeded, damage);


        if (damage >= damageNeeded) {
            uint32_t factor = damage % damageNeeded;
            if (factor > shipQuantity) {
                factor = shipQuantity;
            }
            fleet->removeShips(shipNumber, factor);
            fleet->setDamage(damage - (damageNeeded*factor));
            Logger::getLogger()->debug("Damage is greater than a ship can take, Total(%d), Delivering(%d), factor (%d), new setDamage(%d)",
                damage, damageNeeded*factor, factor, fleet->getDamage());
        }
    }

    Logger::getLogger()->debug("AVACombat::resolveCombat : Exiting");


}


// Combat always takes place between two fleets.  If one combatant
// or the other is actually a planet, the planet simulated by two
// battleships.
void AVACombat::doCombat()
{
    Logger::getLogger()->debug("AVACombat::doCombat : Entering");

    Fleet *fleets[2];

    Game* game = Game::getGame();
    DesignStore* ds = game->getDesignStore();
    uint32_t obT_Fleet = game->getObjectTypeManager()->getObjectTypeByName("Fleet");
    uint32_t obT_Planet = game->getObjectTypeManager()->getObjectTypeByName("Planet");

    // If one combatant or the other is actually a planet,
    // simulate this with two battleships.
    if ( c1->getType() == obT_Fleet) {
        fleets[0] = dynamic_cast<Fleet*>( c1->getObjectBehaviour());
    }
    else if ( c1->getType() == obT_Planet) {
        fleets[0] = new DummyFleet();

        std::set<uint32_t> dIDs = ds->getDesignIds();
        uint32_t scoutID=0;
        for (std::set<uint32_t>::iterator it = dIDs.begin(); it != dIDs.end(); ++it) {
            if (ds->getDesign(*it)->getName() == "Scout") {
                scoutID = *it;
            }
        }
        fleets[0] = new DummyFleet();
        fleets[0]->addShips(scoutID , 2);
        fleets[0]->setOwner( dynamic_cast<Planet*>(c1->getObjectBehaviour())->getOwner());
    }
    if ( c2->getType() == obT_Fleet) {
        fleets[1] = dynamic_cast<Fleet*>( c2->getObjectBehaviour());
    }
    else if ( c2->getType() == obT_Planet) {
        std::set<uint32_t> dIDs = ds->getDesignIds();
        uint32_t scoutID=0;
        for (std::set<uint32_t>::iterator it = dIDs.begin(); it != dIDs.end(); ++it) {
            if (ds->getDesign(*it)->getName() == "Scout") {
                scoutID = *it;
            }
        }
        fleets[1] = new DummyFleet();
        fleets[1]->addShips(scoutID, 2);
        fleets[1]->setOwner( dynamic_cast<Planet*>(c2->getObjectBehaviour())->getOwner());
    }
    if ( fleets[0] == NULL || fleets[1] == NULL) {
        return;
    }

    Message *msgs[2];
    msgs[0] = new Message();
    msgs[1] = new Message();
    msgs[0]->setSubject( "Combat");
    msgs[1]->setSubject( "Combat");
    msgs[0]->addReference( rst_Object, c1->getID());
    msgs[0]->addReference( rst_Object, c2->getID());
    msgs[0]->addReference( rst_Player, fleets[1]->getOwner());
    msgs[1]->addReference( rst_Object, c2->getID());
    msgs[1]->addReference( rst_Object, c1->getID());
    msgs[1]->addReference( rst_Player, fleets[0]->getOwner());

    while ( doCombatRound( fleets, msgs )) {
        ;
    }

    Game::getGame()->getPlayerManager()->getPlayer( fleets[0]->getOwner())->postToBoard( msgs[0]);
    Game::getGame()->getPlayerManager()->getPlayer( fleets[1]->getOwner())->postToBoard( msgs[1]);

    return;
}

}
