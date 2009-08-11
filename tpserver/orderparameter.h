#ifndef ORDERPARAMETER_H
#define ORDERPARAMETER_H
/*  OrderParameter base class
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

#include <tpserver/describable.h>
#include <tpserver/packable.h>
#include <tpserver/inputframe.h>

// Used only by persistence
typedef enum {
	opT_Invalid = -1,
	opT_Space_Coord_Abs = 0,
	opT_Time = 1,
	opT_Object_ID = 2,
	opT_Player_ID = 3,
	opT_Space_Coord_Rel = 4,
	opT_Range = 5,
	opT_List = 6,
	opT_String = 7,

	opT_Max
} OrderParamType;

class Frame;

class OrderParameter : public Describable, public Packable {
  public:
    OrderParameter(const std::string& aname, const std::string& adesc );
    virtual ~OrderParameter();

    // used only by persistence
    uint32_t getType() const;
    void packOrderDescFrame(OutputFrame* f) const;
    virtual bool unpack(InputFrame * f) = 0;

};

#endif
