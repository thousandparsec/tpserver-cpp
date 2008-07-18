#ifndef FLEET_H
#define FLEET_H
/*  Fleet Object class
 *
 *  Copyright (C) 2008 Dustin White and the Thousand Parsec Project
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <list>
#include <string>

#include <tpserver/vector3d.h>
#include "ownedobject.h"

class IGObject;

class FleetType : public OwnedObjectType{
    public:
        FleetType();
        virtual ~FleetType();

    protected:
        ObjectBehaviour* createObjectBehaviour() const;
};

class Fleet : public OwnedObject {
    public:
        Fleet();
        virtual ~Fleet();


        void setDefaultOrderTypes();
        void addAllowedOrder(std::string order);
        void addAllowedCombatOrder(std::string order);
        void toggleCombat();
        void addShips(uint32_t type, uint32_t number);
        bool removeShips(uint32_t type, uint32_t number);
        uint32_t numShips(uint32_t type);
        std::map<uint32_t, uint32_t> getShips() const;
        uint32_t totalShips() const;

        uint32_t getDamage() const;
        void setDamage(uint32_t nd);

        void packExtraData(Frame * frame);

        void doOnceATurn();
        void setupObject();

        int getContainerType();
    private:
        bool combat;
        std::set<uint32_t> normalOrders;
        std::set<uint32_t> combatOrders;
};

#endif
