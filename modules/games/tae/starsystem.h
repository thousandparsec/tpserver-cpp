#ifndef STAR_SYSTEM_H
#define STAR_SYSTEM_H
/*  Star System class
 *
 *  Copyright (C) 2008 Dustin White and the Thousand Parsec Project
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

#include <string>

#include "spaceobject.h"

class StarSystemType : public SpaceObjectType {
    public:
        StarSystemType();
        virtual ~StarSystemType();
    
        void setTypeName(const std::string& n);
        void setTypeDescription(const std::string& d);

    protected:
        ObjectBehaviour* createObjectBehaviour() const;
};

class StarSystem : public SpaceObject {
    public:
        StarSystem();
        virtual ~StarSystem();

        uint32_t getRegion();
        void setRegion(uint32_t region);
        bool canBeColonized(bool mining);
        bool isInhabitable();
        void setInhabitable(bool inhabit);
        bool isDestroyed();
        void setDestroyed(bool des);

        virtual void doOnceATurn();
        virtual int getContainerType();

};



#endif
