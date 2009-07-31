/*  The FleetBuilder class.  This class is used to create and manage
 *  fleets.  It keeps track of how many fleets of each type are created
 *  since there are a limited number of colonist fleets.  It also provides
 *  simple functions to generate new fleets when needed.
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

#include <ctime>

#include <tpserver/game.h>
#include <tpserver/orderqueue.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/objectview.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/designview.h>
#include <tpserver/player.h>
#include <tpserver/playermanager.h>
#include <tpserver/playerview.h>

#include "fleet.h"
#include "taeturn.h"

#include "fleetbuilder.h"

using std::map;

/*
 * PUBLIC FUNCTIONS
 */

FleetBuilder::FleetBuilder() {
    std::srand(std::time(NULL));

    //Set initial ship reserve
    shipsLeft[MERCHANT_SHIP] = 30;
    shipsLeft[SCIENTIST_SHIP] = 57;
    shipsLeft[SETTLER_SHIP] = 30;
    shipsLeft[MINING_SHIP] = 36;
}

FleetBuilder::~FleetBuilder() {
}

//Creates a fleet of type fleetType with ships of shipType for the owner at the location of the parent
//with the name of name
IGObject* FleetBuilder::createFleet(int fleetType, int shipType, Player* owner, IGObject* parent, std::string name) {
    Game *game = Game::getGame();
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    IGObject *fleet = game->getObjectManager()->createNewObject();

    //Create Fleet
    obtm->setupObject(fleet, obtm->getObjectTypeByName("Fleet"));

    Fleet* theFleet = (Fleet*) (fleet->getObjectBehaviour());
    theFleet->setSize(2);
    fleet->setName(name.c_str());
    theFleet->setOwner(owner->getID());

    theFleet->setPosition(((SpaceObject*)(parent->getObjectBehaviour()))->getPosition());
    theFleet->setVelocity(Vector3d(0ll,0ll,0ll));

    OrderQueue *fleetoq = new OrderQueue();
    fleetoq->setQueueId(fleet->getID());
    fleetoq->addOwner(owner->getID());
    game->getOrderManager()->addOrderQueue(fleetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(fleet->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(fleetoq->getQueueId());
    theFleet->setDefaultOrderTypes();

    fleet->addToParent(parent->getID());

    //Add ship
    Design* ship;
    if(fleetType == PASSENGER_FLEET) {
        if(shipType == RANDOM_SHIP) {
            ship = createRandomPassengerShip(owner);
        } else {
            ship = createPassengerShip(owner, shipType);
        }

        if(ship->getName().compare("MiningShip") == 0) {
            theFleet->addAllowedOrder("ColonizeMining");
        } else {
            theFleet->addAllowedOrder("Colonize");
        }
        theFleet->addAllowedCombatOrder("Reinforce");
    } else if(fleetType == VIP_FLEET) {
        ship = createVIPTransport(owner, shipType);
        theFleet->addAllowedOrder("Move");
    } else {
        ship = createBomber(owner);
        theFleet->addAllowedOrder("Attack");
    }
    theFleet->addShips(ship->getDesignId(), 1);
    game->getDesignStore()->designCountsUpdated(ship);

    std::set<uint32_t> playerids = game->getPlayerManager()->getAllIds();
    for(std::set<uint32_t>::iterator playerit = playerids.begin(); playerit != playerids.end(); ++playerit){
        Player* player = game->getPlayerManager()->getPlayer(*playerit);
        if(*playerit == owner->getID()) {
            player->getPlayerView()->addVisibleDesign( ship->getDesignId(), true );
        } else {
        //Colonist ship types are not viewable by enemy players
            player->getPlayerView()->addVisibleDesign( ship->getDesignId(), fleetType != PASSENGER_FLEET );
        }

        player->getPlayerView()->addVisibleObject( fleet->getID(), true );

        game->getPlayerManager()->updatePlayer(player->getID());
    }


    return fleet;
}

//Are there any ships left?
bool FleetBuilder::shipsEmpty() {
    int total = 0;
    for(int i =0; i < 4; i++) {
        total += shipsLeft[i];
    }
    return (total <= 0);
}


/*
 * PRIVATE FUNCTIONS
 */

//Create a passenger ship of the specified type for the owner
Design* FleetBuilder::createPassengerShip(Player* owner, int type) {
    Design* ship = new Design();
    IdMap componentList;

    DesignStore * ds = Game::getGame()->getDesignStore();

    ship->setCategoryId(ds->getCategoryByName("Ships"));
    ship->setDescription("A passenger transport ship");
    ship->setOwner(owner->getID());

    if(type == MERCHANT_SHIP) {
        ship->setName("MerchantShip");
        componentList[ds->getComponentByName("MerchantCargo")] = 1;
    } else if (type == SCIENTIST_SHIP) {
        ship->setName("ScientistShip");
        componentList[ds->getComponentByName("ScientistCargo")] = 1;
    } else if (type == SETTLER_SHIP) {
        ship->setName("SettlerShip");
        componentList[ds->getComponentByName("SettlerCargo")] = 1;
    } else {
        ship->setName("MiningShip");
        componentList[ds->getComponentByName("MiningCargo")] = 1;
    }
    ship->setComponents(componentList);
    ds->addDesign(ship);

    return ship;
}

//Create a random passenger ship
Design* FleetBuilder::createRandomPassengerShip(Player* owner) {
    int type;

    //Check to see if there are any ships left
    if(shipsEmpty()) {
        TaeTurn* turn = (TaeTurn*) Game::getGame()->getTurnProcess();
        turn->setGameOver(true);
    }

    //Select a ship type
    do {
        type = std::rand() % 4;
    } while(shipsLeft[type] <= 0);

    shipsLeft[type]--;

    return createPassengerShip(owner, type);        
}

//Create a leader's ship
Design* FleetBuilder::createVIPTransport(Player* owner, int type) {
    Design* ship = new Design();
    IdMap componentList;

    DesignStore * ds = Game::getGame()->getDesignStore();

    ship->setCategoryId(ds->getCategoryByName("Ships"));
    ship->setDescription("A passenger transport ship for VIPs");
    ship->setOwner(owner->getID());
    if(type == MERCHANT_SHIP) {
        ship->setName("MerchantLeaderShip");
        componentList[ds->getComponentByName("MerchantLeaderCargo")] = 1;
    } else if (type == SCIENTIST_SHIP) {
        ship->setName("ScientistLeaderShip");
        componentList[ds->getComponentByName("ScientistLeaderCargo")] = 1;
    } else if (type == SETTLER_SHIP) {
        ship->setName("SettlerLeaderShip");
        componentList[ds->getComponentByName("SettlerLeaderCargo")] = 1;
    } else {
        ship->setName("MiningLeaderShip");
        componentList[ds->getComponentByName("MiningLeaderCargo")] = 1;
    }
    ship->setComponents(componentList);
    ds->addDesign(ship);

    return ship;

}

//Create a bomber
Design* FleetBuilder::createBomber(Player* owner) {
    Design* ship = new Design();
    IdMap componentList;

    DesignStore * ds = Game::getGame()->getDesignStore();

    ship->setCategoryId(ds->getCategoryByName("Ships"));
    ship->setName("Bomber");
    ship->setDescription("A bomber capable of destroying star systems");
    ship->setOwner(owner->getID());
    componentList[ds->getComponentByName("Weapon")] = 1;
    ship->setComponents(componentList);
    ds->addDesign(ship);

    return ship;
}
