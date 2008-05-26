#ifndef PLANET_H
#define PLANET_H
/*  Planet class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project 
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/vector3d.h>

#include "staticobject.h"
#include "ownedobject.h"

namespace RiskRuleset {

class PlanetType : public StaticObjectType {
  public:
    PlanetType();
    virtual ~PlanetType(){}

  protected:
    ObjectBehaviour* createObjectBehaviour() const;
};

class Planet : public StaticObject, public OwnedObject {

  public:
    Planet();
    virtual ~Planet();

    virtual void packExtraData(Frame * frame);
    virtual void doOnceATurn();
    virtual int getContainerType();

    uint32_t getOwner() const;
    void setOwner(uint32_t no);
    
    int getArmies() const;
    void setArmies(int numArmies);

    void setOrderTypes();

    void setupObject();

};


}

#endif
