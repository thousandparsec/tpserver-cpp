#ifndef OWNEDOBJECT_H
#define OWNEDOBJECT_H
/*  Owned Object class
 *
 *  Copyright (C) 2004-2005, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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

#include "spaceobject.h"

class IGObject;

class OwnedObjectType : public SpaceObjectType{
  public:
    OwnedObjectType( const std::string& nname, const std::string& ndesc );
    virtual ~OwnedObjectType();
};

class OwnedObject : public SpaceObject {
      public:
	OwnedObject();
	virtual ~OwnedObject();
        
        uint32_t getOwner() const;
        void setOwner(uint32_t no);

        virtual void setDefaultOrderTypes() = 0;
	
	void packExtraData(OutputFrame * frame);


        virtual void setupObject();
        
    protected:
        static const uint32_t OWNERGRPID;
        static const uint32_t OWNERPARAMID;
        static const uint32_t ORDERGRPID;
        static const uint32_t ORDERQPARAMID;

};

#endif
