#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H
/*  PlayerView class
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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
class Component;

struct ModListItem{
  ModListItem();
  ModListItem(uint32_t nid, uint64_t nmt);
  uint32_t id;
  uint64_t modtime;
};

class PlayerView {
public:
  PlayerView();
  ~PlayerView();

  void setPlayerId(uint32_t newid);
  
  void doOnceATurn();

  void setVisibleObjects(std::set<unsigned int> vis);
  bool isVisibleObject(unsigned int objid);
  std::set<uint32_t> getVisibleObjects() const;

  void addVisibleDesign(unsigned int designid);
  void addUsableDesign(unsigned int designid);
  void removeUsableDesign(unsigned int designid);
  bool isUsableDesign(unsigned int designid) const;
  std::set<unsigned int> getUsableDesigns() const;
  std::set<uint32_t> getVisibleDesigns() const;

  void addVisibleComponent(Component* comp);
  void addUsableComponent(uint32_t compid);
  void removeUsableComponent(uint32_t compid);
  bool isUsableComponent(uint32_t compid) const;
  std::set<uint32_t> getVisibleComponents() const;
  std::set<uint32_t> getUsableComponents() const;
  void processGetComponent(uint32_t compid, Frame* frame) const;
  void processGetComponentIds(Frame* in, Frame* out) const;

  uint32_t getObjectSequenceKey() const;

private:
  uint32_t pid;

  std::set<uint32_t> visibleObjects;
  uint32_t currObjSeq;

  std::set<uint32_t> visibleDesigns;
  std::set<uint32_t> usableDesigns;

  std::set<uint32_t> visibleComponents;
  std::set<uint32_t> usableComponents;
  std::map<uint32_t, Component*> cacheComponents;
  std::list<ModListItem> difflistComponents;
  std::map<uint32_t, ModListItem> turnCompdifflist;
  uint32_t currCompSeq;

};

#endif
