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

#include <list>
#include <stdint.h>
#include <tpserver/common.h>

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
  void addVisibleObjects( const IdSet& obids );
  ObjectView* getObjectView(uint32_t objid);
  void updateObjectView(uint32_t objid);
  void removeVisibleObject(uint32_t objid);
  bool isVisibleObject(uint32_t objid);
  IdSet getVisibleObjects() const;
  void addOwnedObject(uint32_t objid);
  void removeOwnedObject(uint32_t objid);
  uint32_t getNumberOwnedObjects() const;
  IdSet getOwnedObjects() const;
  void processGetObject(uint32_t objid, Frame* frame);
  void processGetObjectIds(Frame* in, Frame* out);

  void addVisibleDesign(DesignView* design);
  void addVisibleDesigns( const IdSet& obids );
  void addUsableDesign(uint32_t designid);
  void removeUsableDesign(uint32_t designid);
  bool isUsableDesign(uint32_t designid) const;
  IdSet getUsableDesigns() const;
  IdSet getVisibleDesigns() const;
  void processGetDesign(uint32_t designid, Frame* frame);
  void processGetDesignIds(Frame* in, Frame* out);

  void addVisibleComponent(ComponentView* comp);
  void addVisibleComponents( const IdSet& obids );
  void addUsableComponent(uint32_t compid);
  void removeUsableComponent(uint32_t compid);
  bool isUsableComponent(uint32_t compid) const;
  IdSet getVisibleComponents() const;
  IdSet getUsableComponents() const;
  void processGetComponent(uint32_t compid, Frame* frame);
  void processGetComponentIds(Frame* in, Frame* out);
  
  //for persistence only!
  void setVisibleObjects(const IdSet& obids);
  void setOwnedObjects(const IdSet& obids);
  void setVisibleDesigns(const IdSet& dids);
  void setUsableDesigns(const IdSet& dids);
  void setVisibleComponents(const IdSet& cids);
  void setUsableComponents(const IdSet& cids);

private:
  uint32_t pid;

  // TODO: modify to be based on interfaces not templates
  template< class EntityType >
  struct EntityInfo {
    IdSet visible;
    IdSet actable;
    std::map<uint32_t, EntityType*> cache;
    IdModList modified;
    uint32_t sequence;
    uint32_t pid;
    EntityInfo() : sequence( 0 ) {}
    void packEntityList( Frame* out, FrameType type, uint32_t snum, uint32_t numtoget, uint64_t fromtime );
    void addVisible( EntityType* entity );
    void addVisible( const IdSet& obids );
    void addActable( uint32_t id );
    void removeActable( uint32_t id );
    bool isActable( uint32_t id ) const;
    bool isVisible( uint32_t id ) const;
    EntityType* retrieve( uint32_t id );
  };

  EntityInfo< ObjectView > objects;
  EntityInfo< DesignView > designs;
  EntityInfo< ComponentView > components;

};

#endif
