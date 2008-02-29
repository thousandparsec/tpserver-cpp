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

class Frame;
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
  ObjectView* getObjectView(uint32_t objid) const;
  void removeVisibleObject(uint32_t objid);
  bool isVisibleObject(unsigned int objid) const;
  std::set<uint32_t> getVisibleObjects() const;
  void addOwnedObject(uint32_t objid);
  void removeOwnedObject(uint32_t objid);
  uint32_t getNumberOwnedObjects() const;
  std::set<uint32_t> getOwnedObjects() const;
  void processGetObject(uint32_t objid, Frame* frame) const;
  void processGetObjectIds(Frame* in, Frame* out);

  void addVisibleDesign(DesignView* design);
  void addUsableDesign(uint32_t designid);
  void removeUsableDesign(uint32_t designid);
  bool isUsableDesign(uint32_t designid) const;
  std::set<uint32_t> getUsableDesigns() const;
  std::set<uint32_t> getVisibleDesigns() const;
  void processGetDesign(uint32_t designid, Frame* frame) const;
  void processGetDesignIds(Frame* in, Frame* out);

  void addVisibleComponent(ComponentView* comp);
  void addUsableComponent(uint32_t compid);
  void removeUsableComponent(uint32_t compid);
  bool isUsableComponent(uint32_t compid) const;
  std::set<uint32_t> getVisibleComponents() const;
  std::set<uint32_t> getUsableComponents() const;
  void processGetComponent(uint32_t compid, Frame* frame) const;
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

  std::set<uint32_t> visibleObjects;
  std::set<uint32_t> ownedObjects;
  std::map<uint32_t, ObjectView*> cacheObjects;
  std::map<uint32_t, uint64_t> modlistObject;
  uint32_t currObjSeq;

  std::set<uint32_t> visibleDesigns;
  std::set<uint32_t> usableDesigns;
  std::map<uint32_t, DesignView*> cacheDesigns;
  std::map<uint32_t, uint64_t> modlistDesign;
  uint32_t currDesignSeq;

  std::set<uint32_t> visibleComponents;
  std::set<uint32_t> usableComponents;
  std::map<uint32_t, ComponentView*> cacheComponents;
  std::map<uint32_t, uint64_t> modlistComp;
  uint32_t currCompSeq;

};

#endif
