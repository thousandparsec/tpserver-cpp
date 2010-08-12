#ifndef PLANET_H
#define PLANET_H
/*  Planet ObjectData class
 *
 *  Copyright (C) 2004, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/vector3d.h>
#include <tpserver/object.h>

#include "ownedobject.h"
#include "emptyobject.h"

class PlanetType : public OwnedObjectType{
  public:
    PlanetType();
    virtual ~PlanetType();
    
  protected:
    ObjectBehaviour* createObjectBehaviour() const;
  
};

class Planet:public OwnedObject {
      public:
	Planet();
        virtual ~Planet();

        void setDefaultOrderTypes();
        
	void packExtraData(OutputFrame::Ptr frame);

	void doOnceATurn();

	int getContainerType();
	

    std::map<uint32_t, std::pair<uint32_t, uint32_t> > getResources();
    uint32_t getResource(uint32_t restype) const;
    
    void setResources(std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress);
    void addResource(uint32_t restype, uint32_t amount);
    bool removeResource(uint32_t restype, uint32_t amount);

    static IGObject::Ptr createObject(IGObject::Ptr parent, std::string name, Vector3d v, uint32_t size, std::string media);

    private:
        static const uint32_t RESGRPID;
        static const uint32_t RESPARAMID;

};

#endif
