#ifndef OBJECTDATAMANAGER_H
#define OBJECTDATAMANAGER_H
/*  ObjectDataManager class
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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

typedef enum{
  obT_Invalid = -1,
  obT_Universe = 0,
  obT_Galaxy = 1,
  obT_Star_System = 2,
  obT_Planet = 3,
  obT_Fleet = 4,

  obT_Max
} bi_ObjectType;

class ObjectData;

class ObjectDataManager{
 public:
  ObjectDataManager();
  ~ObjectDataManager();

  ObjectData* createObjectData(int type);
  bool checkValid(int type);

  int addNewObjectType(ObjectData* od);

 private:
  std::map<int, ObjectData*> prototypeStore;
  int nextType;

};

#endif
