#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H
/*  PlayerView class
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

#include <set>
#include <map>
#include <list>
#include <string>
#include <stdint.h>
#include <tpserver/protocol.h>

class ObjectView;
class DesignView;
class ComponentView;

class PlayerView {
public:
  PlayerView();
  ~PlayerView();

  void setPlayerId(uint32_t newid);
  
  void doOnceATurn();

  void addVisibleObject(ObjectView* obj);
  ObjectView* getObjectView(uint32_t objid);
  void updateObjectView(uint32_t objid);
  void removeVisibleObject(uint32_t objid);
  bool isVisibleObject(uint32_t objid);
  std::set<uint32_t> getVisibleObjects() const;
  void addOwnedObject(uint32_t objid);
  void removeOwnedObject(uint32_t objid);
  uint32_t getNumberOwnedObjects() const;
  std::set<uint32_t> getOwnedObjects() const;
  void processGetObject(uint32_t objid, Frame* frame);
  void processGetObjectIds(Frame* in, Frame* out);

  void addVisibleDesign(DesignView* design);
  void addUsableDesign(uint32_t designid);
  void removeUsableDesign(uint32_t designid);
  bool isUsableDesign(uint32_t designid) const;
  std::set<uint32_t> getUsableDesigns() const;
  std::set<uint32_t> getVisibleDesigns() const;
  void processGetDesign(uint32_t designid, Frame* frame);
  void processGetDesignIds(Frame* in, Frame* out);

  void addVisibleComponent(ComponentView* comp);
  void addUsableComponent(uint32_t compid);
  void removeUsableComponent(uint32_t compid);
  bool isUsableComponent(uint32_t compid) const;
  std::set<uint32_t> getVisibleComponents() const;
  std::set<uint32_t> getUsableComponents() const;
  void processGetComponent(uint32_t compid, Frame* frame);
  void processGetComponentIds(Frame* in, Frame* out);
  
  //for persistence only!
  void setVisibleObjects(const std::set<uint32_t>& obids);
  void setOwnedObjects(const std::set<uint32_t>& obids);
  void setVisibleDesigns(const std::set<uint32_t>& dids);
  void setUsableDesigns(const std::set<uint32_t>& dids);
  void setVisibleComponents(const std::set<uint32_t>& cids);
  void setUsableComponents(const std::set<uint32_t>& cids);

private:
  uint32_t pid;

  // TODO: modify persistence to make code common for all entity types
  template< class EntityType >
  struct EntityInfo {
    std::set<uint32_t> visible;
    std::set<uint32_t> actable;
    std::map<uint32_t, EntityType*> cache;
    std::map<uint32_t, uint64_t> modified;
    uint32_t sequence;
    EntityInfo() : sequence( 0 ) {}
    void packEntityList( Frame* out, FrameType type, uint32_t snum, uint32_t numtoget, uint64_t fromtime );
  };

  EntityInfo< ObjectView > objects;
  EntityInfo< DesignView > designs;
  EntityInfo< ComponentView > components;

};

#endif
