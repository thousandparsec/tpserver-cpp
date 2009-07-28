#ifndef OBJECTTYPEMANAGER_H
#define OBJECTTYPEMANAGER_H
/*  ObjectTypeManager class
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

#include <tpserver/common.h>

class Object;
class ObjectType;
class Frame;

class ObjectTypeManager{
 public:
  ObjectTypeManager();
  ~ObjectTypeManager();

  void setupObject(IGObject* obj, uint32_t type);
  bool checkValid(uint32_t type);
  
  uint32_t getObjectTypeByName(const std::string& name) const;

  uint32_t addNewObjectType(ObjectType* od);

  uint32_t getSeqKey() const { return seqkey; }
  IdModList getTypeModList(uint64_t fromtime) const;
  void doGetObjectDesc(uint32_t type, Frame* of);

 private:
  std::map<uint32_t, ObjectType*> typeStore;
  std::map<std::string, uint32_t> stringmap;
  uint32_t nextType;
  // TODO: is this really needed?
  uint32_t seqkey;

};

#endif
