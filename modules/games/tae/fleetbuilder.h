#ifndef FLEETBUILDER_H
#define FLEETBUILDER_H
/*  The FleetBuilder class.  A helper class for replenishing fleets.
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

#include <tpserver/design.h>
#include <tpserver/object.h>
#include <tpserver/player.h>
#include <tpserver/game.h>

class FleetBuilder{
    public:
        const static int PASSENGER_FLEET = 10;
        const static int VIP_FLEET = 20;
        const static int BOMBER_FLEET = 30;
        const static int MERCHANT_SHIP = 0;
        const static int SCIENTIST_SHIP = 1;
        const static int SETTLER_SHIP = 2;
        const static int MINING_SHIP = 3;
        const static int RANDOM_SHIP = 4;

        FleetBuilder();
        ~FleetBuilder();

        IGObject* createFleet(int fleetType, int shipType, Player* owner, IGObject* parent, std::string name);
        bool shipsEmpty();
    
    private:
        Design* createPassengerShip(Player* owner, int type);
        Design* createRandomPassengerShip(Player* owner);
        Design* createVIPTransport(Player* owner, int type);
        Design* createBomber(Player* owner);
        int shipsLeft[4];
};

#endif
