#ifndef FLEETBUILDER_H
#define FLEETBUILDER_H
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

#include <tpserver/design.h>
#include <tpserver/object.h>
#include <tpserver/player.h>
#include <tpserver/game.h>

class FleetBuilder{
    public:
        //These static variables are used to indicate which types of
        //fleets and ships are to be created by the functions
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

        //Creates a fleet of type fleetType with ships of shipType for the owner at the location of the parent
        //with the name of name
        IGObject::Ptr createFleet(int fleetType, int shipType, Player::Ptr owner, IGObject::Ptr parent, std::string name);
        //This function returns true if no more colonist fleets can be created
        bool shipsEmpty();
    
    private:
        //Private functions used to create the different fleets
        Design::Ptr  createPassengerShip(Player::Ptr owner, int type);
        Design::Ptr  createRandomPassengerShip(Player::Ptr owner);
        Design::Ptr  createVIPTransport(Player::Ptr owner, int type);
        Design::Ptr  createBomber(Player::Ptr owner);

        //Used to keep track of how many colonist ships are left for each type
        int shipsLeft[4];
};

#endif
