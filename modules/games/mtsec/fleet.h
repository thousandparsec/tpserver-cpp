#ifndef FLEET_H
#define FLEET_H
/*  Fleet Object class
 *
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

#include <tpserver/vector3d.h>
#include "ownedobject.h"

class IGObject;

namespace MTSecRuleset {

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
	virtual void addShips(uint32_t type, uint32_t number);
	virtual bool removeShips(uint32_t type, uint32_t number);
	virtual uint32_t numShips(uint32_t type);
	virtual std::map<uint32_t, uint32_t> getShips() const;
	virtual uint32_t totalShips() const;
	int64_t maxSpeed();
    int32_t getDesignId(uint32_t id) const;


        virtual uint32_t getDamage() const;
        virtual void setDamage(uint32_t nd);

	void packExtraData(OutputFrame::Ptr frame);

	void doOnceATurn();
        void setupObject();

	int getContainerType();

        void addResource(uint32_t restype, uint32_t amount);
        void setResource(uint32_t restype, uint32_t amount);
        virtual bool removeResource(uint32_t restype, uint32_t amount);
        virtual std::map<uint32_t, std::pair<uint32_t, uint32_t> > getResources();

      protected:
};

}
#endif

