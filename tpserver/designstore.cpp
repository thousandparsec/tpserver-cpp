/*  DesignStore class
 *
 *  Copyright (C) 2005,2007  Lee Begg and the Thousand Parsec Project
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


#include "category.h"
#include "design.h"
#include "designview.h"
#include "component.h"
#include "property.h"
#include "game.h"
#include "player.h"
#include "playerview.h"
#include "playermanager.h"
#include "persistence.h"

#include "designstore.h"

DesignStore::DesignStore(){
  next_designid = 1;
  next_componentid = 1;
  next_propertyid = 1;
  next_categoryid = 1;
}

DesignStore::~DesignStore(){
  while(!designs.empty()){
    delete designs.begin()->second;
    designs.erase(designs.begin());
  }
  while(!components.empty()){
    delete components.begin()->second;
    components.erase(components.begin());
  }
  while(!properties.empty()){
    delete properties.begin()->second;
    properties.erase(properties.begin());
  }
}

void DesignStore::init(){
  Persistence* persistence = Game::getGame()->getPersistence();
  next_designid = persistence->getMaxDesignId() + 1;
  next_componentid = persistence->getMaxComponentId() + 1;
  next_propertyid = persistence->getMaxPropertyId() + 1;
  next_categoryid = persistence->getMaxCategoryId() + 1;
  IdSet ids = persistence->getCategoryIds();
  for(IdSet::iterator itcurr = ids.begin(); itcurr != ids.end(); ++itcurr){
    categories[*itcurr] = NULL;
  }
  ids = persistence->getDesignIds();
  for(IdSet::iterator itcurr = ids.begin(); itcurr != ids.end(); ++itcurr){
    designs[*itcurr] = NULL;
  }
  ids = persistence->getComponentIds();
  for(IdSet::iterator itcurr = ids.begin(); itcurr != ids.end(); ++itcurr){
    components[*itcurr] = NULL;
    Component* tmpcomp = persistence->retrieveComponent(*itcurr);
    componentIndex[tmpcomp->getName()] = (*itcurr);
    delete tmpcomp;
  }
  ids = persistence->getPropertyIds();
  for(IdSet::iterator itcurr = ids.begin(); itcurr != ids.end(); ++itcurr){
    properties[*itcurr] = persistence->retrieveProperty(*itcurr);
    propertyIndex[properties[*itcurr]->getName()] = (*itcurr);
  }
}

Category* DesignStore::getCategory(uint32_t id){
  std::map<uint32_t, Category*>::iterator pos = categories.find(id);
  Category* cat = NULL;
  if(pos != categories.end()){
    cat = pos->second;
    if(cat == NULL){
      cat = Game::getGame()->getPersistence()->retrieveCategory(id);
      pos->second = cat;
    }
  }
  return cat;
}

Design* DesignStore::getDesign(uint32_t id){
  std::map<uint32_t, Design*>::iterator pos = designs.find(id);
  Design* design = NULL;
  if(pos != designs.end()){
    design = pos->second;
    if(design == NULL){
      design = Game::getGame()->getPersistence()->retrieveDesign(id);
      pos->second = design;
    }
  }
  return design;
}

Component* DesignStore::getComponent(uint32_t id){
  std::map<uint32_t, Component*>::iterator pos = components.find(id);
  Component* comp = NULL;
  if(pos != components.end()){
    comp = pos->second;
    if(comp == NULL){
      comp = Game::getGame()->getPersistence()->retrieveComponent(id);
      pos->second = comp;
    }
  }
  return comp;
}

Property* DesignStore::getProperty(uint32_t id){
  std::map<uint32_t, Property*>::iterator pos = properties.find(id);
  Property* prop = NULL;
  if(pos != properties.end()){
    prop = pos->second;
    if(prop == NULL){
      prop = Game::getGame()->getPersistence()->retrieveProperty(id);
      pos->second = prop;
      propertyIndex[prop->getName()] = prop->getPropertyId();
    }
  }
  return prop;
}


IdSet DesignStore::getCategoryIds() const{
  IdSet set;
  for(std::map<uint32_t, Category*>::const_iterator itcurr = categories.begin();
      itcurr != categories.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

IdSet DesignStore::getDesignIds() const{
  IdSet set;
  for(std::map<uint32_t, Design*>::const_iterator itcurr = designs.begin();
      itcurr != designs.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

IdSet DesignStore::getComponentIds() const{
  IdSet set;
  for(std::map<uint32_t, Component*>::const_iterator itcurr = components.begin();
      itcurr != components.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

IdSet DesignStore::getPropertyIds() const{
  IdSet set;
  for(std::map<uint32_t, Property*>::const_iterator itcurr = properties.begin();
      itcurr != properties.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

void DesignStore::addCategory(Category* c){
  c->setCategoryId(next_categoryid++);
  categories[c->getId()] = c;
  categoryIndex[c->getName()] = c->getId();
  Game::getGame()->getPersistence()->saveCategory(c);
}

bool DesignStore::addDesign(Design* d){
  d->setDesignId(next_designid++);

  //check components all come from this category
  IdMap cl = d->getComponents();
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(d->getOwner());
  PlayerView::Ptr playerview = player->getPlayerView();
  for(IdMap::iterator itcurr = cl.begin(); 
      itcurr != cl.end(); ++itcurr){
    if(!(playerview->isUsableComponent(itcurr->first)))
      return false;
    std::map<uint32_t, Component*>::iterator itcomp = components.find(itcurr->first);
    if(itcomp == components.end())
      return false;
    Component* comp = getComponent(itcurr->first);
    comp->setInUse();
  }
  d->eval();
  designs[d->getId()] = d;
  getCategory(d->getCategoryId())->doAddDesign(d);

  if(d->isValid()){
    playerview->addUsableDesign(d->getId());
  }else{
    playerview->addVisibleDesign( new DesignView( d->getId(), true ) );
  }

  Game::getGame()->getPlayerManager()->updatePlayer(player->getID());
  Game::getGame()->getPersistence()->saveDesign(d);
  return true;
}

bool DesignStore::modifyDesign(Design* d){
  Design* current = designs[d->getId()];
  if(current == NULL || current->getOwner() != d->getOwner() || current->getNumExist() != 0 || current->getInUse() != 0)
    return false;
  Player* player = Game::getGame()->getPlayerManager()->getPlayer(d->getOwner());
  PlayerView::Ptr playerview = player->getPlayerView();
  playerview->removeUsableDesign(d->getId());

  IdMap cl = current->getComponents();
  for(IdMap::iterator itcurr = cl.begin(); itcurr != cl.end(); ++itcurr){
    Component* comp = components[itcurr->first];
    comp->setInUse(false);
  }
  for(IdMap::iterator itcurr = cl.begin(); itcurr != cl.end(); ++itcurr){
    if(!(playerview->isUsableComponent(itcurr->first)))
      return false;
    std::map<uint32_t, Component*>::iterator itcomp = components.find(itcurr->first);
    if(itcomp == components.end())
      return false;
    itcomp->second->setInUse();
  }

  d->eval();
  bool rtv;
  if(getCategory(d->getCategoryId())->doModifyDesign(d)){
    designs[d->getId()] = d;
    delete current;
    rtv = true;
  }else{
    delete d;
    d = current;
    rtv = false;
  }
  if(d->isValid()){
    playerview->addUsableDesign(d->getId());
  }
  Game::getGame()->getPlayerManager()->updatePlayer(player->getID());
  Game::getGame()->getPersistence()->updateDesign(d);
  return rtv;
}

void DesignStore::designCountsUpdated(Design* d){
  Game::getGame()->getPersistence()->updateDesign(d);
}

void DesignStore::addComponent(Component* c){
  c->setComponentId(next_componentid++);
  components[c->getId()] = c;
  componentIndex[c->getName()] = c->getId();
  Game::getGame()->getPersistence()->saveComponent(c);
}

void DesignStore::addProperty(Property* p){
  p->setPropertyId(next_propertyid++);
  properties[p->getId()] = p;
  propertyIndex[p->getName()] = p->getId();
  Game::getGame()->getPersistence()->saveProperty(p);
}

uint32_t DesignStore::getCategoryByName(const std::string& name){
  return categoryIndex[name];
}

uint32_t DesignStore::getComponentByName(const std::string& name){
  return componentIndex[name];
}

uint32_t DesignStore::getPropertyByName(const std::string& name){
  return propertyIndex[name];
}

uint32_t DesignStore::getMaxDesignId() const{
  return (next_designid - 1);
}

uint32_t DesignStore::getMaxComponentId() const{
  return (next_componentid - 1);
}

uint32_t DesignStore::getMaxPropertyId() const{
  return (next_propertyid - 1);
}
