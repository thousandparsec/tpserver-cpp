#ifndef OBJECTORDERPARAMETER_H
#define OBJECTORDERPARAMETER_H
/*  ObjectOrderParameter class
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

#include <set>
#include <stdint.h>

#include <tpserver/protocol.h>

#include <tpserver/orderparameter.h>

class ObjectOrderParameter : public OrderParameter{

public:
  ObjectOrderParameter();
  virtual ~ObjectOrderParameter();

  virtual void packOrderFrame(Frame * f);
  virtual bool unpack(Frame * f);

  uint32_t getObjectId() const;
  void setObjectId(uint32_t id);
  
  std::set<objecttypeid_t> getAllowedObjectTypes() const;
  void setAllowedObjectTypes(const std::set<objecttypeid_t>& nots);
  void addAllowedObjectTypes(objecttypeid_t type);

protected:
  uint32_t object;
  std::set<objecttypeid_t> objecttypes;

};

#endif
