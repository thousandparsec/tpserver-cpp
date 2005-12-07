#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H
/* ObjectManager class, looks after the objects that are in play
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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
#include <set>

class IGObject;
class Vector3d;

class ObjectManager{
 public:
    ObjectManager();
    ~ObjectManager();

    IGObject* createNewObject();
    void addObject(IGObject* obj);
    void discardNewObject(IGObject* obj);

    IGObject *getObject(uint32_t id);
    void doneWithObject(uint32_t id);

    void scheduleRemoveObject(uint32_t id);
    void clearRemovedObjects();
    
    std::set<uint32_t> getObjectsByPos(const Vector3d & pos, uint64_t r);
    std::set<uint32_t> getContainerByPos(const Vector3d & pos);
    
    std::set<uint32_t> getAllIds();

 private:
    std::map<uint32_t, IGObject *> objects;
    std::map<uint32_t, uint64_t> objmtime;
    std::set<uint32_t> scheduleRemove;
    uint32_t nextid;
};

#endif
