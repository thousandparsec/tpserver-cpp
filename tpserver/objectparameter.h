#ifndef OBJECTPARAMETER_H
#define OBJECTPARAMETER_H
/*  ObjectParameter base class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

#include <stdint.h>
#include <string>

typedef enum {
	obpT_Invalid = -1,
	obpT_Position_3D = 0,
	obpT_Velocity = 1,
	obpT_Acceleration = 2,
	obpT_Position_Bound = 3,
	obpT_Order_Queue = 4,
	obpT_Resource_List = 5,
	obpT_Reference = 6,
	obpT_Reference_Quantity_List = 7,
        obpT_Integer = 8,
        obpT_Size = 9,
        obpT_Media = 10,

	obpT_Max
} ObjectParamType;

class Frame;

class ObjectParameter {

      public:
	ObjectParameter();
        virtual ~ObjectParameter();

	uint32_t getType() const;
	std::string getName() const;
        std::string getDescription() const;
        
        void setName(const std::string& nn);
        void setDescription(const std::string& nd);
        
	virtual void packObjectFrame(Frame * f, uint32_t playerid) = 0;
        virtual void packObjectDescFrame(Frame* f) const;
	virtual bool unpackModifyObjectFrame(Frame * f, unsigned int playerid) = 0;

        virtual void signalRemoval();

	virtual ObjectParameter *clone() const = 0;

      protected:
	 uint32_t type;
	 std::string name;
         std::string description;

};

#endif
