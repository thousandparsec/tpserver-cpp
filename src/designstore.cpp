/*  DesignStore class
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


#include "category.h"
#include "design.h"
#include "component.h"
#include "property.h"
#include "game.h"
#include "player.h"

#include "designstore.h"

unsigned int DesignStore::next_designid = 1;
unsigned int DesignStore::next_componentid = 1;
unsigned int DesignStore::next_propertyid = 1;
unsigned int DesignStore::next_categoryid = 1;

DesignStore::DesignStore(){
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

Category* DesignStore::getCategory(unsigned int id){
  std::map<unsigned int, Category*>::iterator pos = categories.find(id);
  if(pos != categories.end()){
    return pos->second;
  }else{
    return NULL;
  }
}

Design* DesignStore::getDesign(unsigned int id){
  std::map<unsigned int, Design*>::iterator pos = designs.find(id);
  if(pos != designs.end()){
    return pos->second;
  }else{
    return NULL;
  }
}

Component* DesignStore::getComponent(unsigned int id){
  std::map<unsigned int, Component*>::iterator pos = components.find(id);
  if(pos != components.end()){
    return pos->second;
  }else{
    return NULL;
  }
}

Property* DesignStore::getProperty(unsigned int id){
  std::map<unsigned int, Property*>::iterator pos = properties.find(id);
  if(pos != properties.end()){
    return pos->second;
  }else{
    return NULL;
  }
}


std::set<unsigned int> DesignStore::getCategoryIds() const{
  std::set<unsigned int> set;
  for(std::map<unsigned int, Category*>::const_iterator itcurr = categories.begin();
      itcurr != categories.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

std::set<unsigned int> DesignStore::getDesignIds() const{
  std::set<unsigned int> set;
  for(std::map<unsigned int, Design*>::const_iterator itcurr = designs.begin();
      itcurr != designs.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

std::set<unsigned int> DesignStore::getComponentIds() const{
  std::set<unsigned int> set;
  for(std::map<unsigned int, Component*>::const_iterator itcurr = components.begin();
      itcurr != components.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

std::set<unsigned int> DesignStore::getPropertyIds() const{
  std::set<unsigned int> set;
  for(std::map<unsigned int, Property*>::const_iterator itcurr = properties.begin();
      itcurr != properties.end(); ++itcurr){
    set.insert(itcurr->first);
  }
  return set;
}

void DesignStore::addCategory(Category* c){
  c->setCategoryId(next_categoryid++);
  categories[c->getCategoryId()] = c;
}

bool DesignStore::addDesign(Design* d){
  d->setDesignId(next_designid++);
  
  //check components all come from this category
    std::map<unsigned int, unsigned int> cl = d->getComponents();
    Player* player = Game::getGame()->getPlayer(d->getOwner());
    for(std::map<unsigned int, unsigned int>::iterator itcurr = cl.begin(); 
            itcurr != cl.end(); ++itcurr){
        if(!(player->isUsableComponent(itcurr->first)))
            return false;
        if(components.find(itcurr->first) == components.end())
            return false;
    }
  d->eval();
  designs[d->getDesignId()] = d;
  categories[d->getCategoryId()]->doAddDesign(d);
  player->addVisibleDesign(d->getDesignId());
  if(d->isValid()){
    player->addUsableDesign(d->getDesignId());
  }
  
  return true;
}

bool DesignStore::modifyDesign(Design* d){
  Design* current = designs[d->getDesignId()];
  if(current == NULL || current->getOwner() != d->getOwner() || current->getNumExist() != 0)
    return false;
  Player* player = Game::getGame()->getPlayer(d->getOwner());
  player->removeUsableDesign(d->getDesignId());
  d->eval();
  bool rtv;
  if(categories[d->getCategoryId()]->doModifyDesign(d)){
    designs[d->getDesignId()] = d;
    delete current;
    rtv = true;
  }else{
    delete d;
    d = current;
    rtv = false;
  }
  if(d->isValid()){
    player->addUsableDesign(d->getDesignId());
  }
  return rtv;
}

void DesignStore::addComponent(Component* c){
  c->setComponentId(next_componentid++);
  Property *p = new Property();
  p->setCategoryId(c->getCategoryId());
  p->setRank(0);
  p->setName(c->getName());
  p->setDescription("The number of " + c->getName() + " components in the design");
  p->setTpclDisplayFunction(
      "(lambda (design bits)"
	"(let "
	  "((n (apply + bits)))"
	  "(cons n (string-append (number->string n) \" components\"))))");
  addProperty(p);
  std::map<unsigned int, std::string> pl = c->getPropertyList();
  pl[p->getPropertyId()] = "(lambda (design) 1)";
  c->setPropertyList(pl);
  components[c->getComponentId()] = c;
}

void DesignStore::addProperty(Property* p){
  p->setPropertyId(next_propertyid++);
  properties[p->getPropertyId()] = p;
}

unsigned int DesignStore::getMaxDesignId() const{
  return (next_designid - 1);
}

unsigned int DesignStore::getMaxComponentId() const{
  return (next_componentid - 1);
}

unsigned int DesignStore::getMaxPropertyId() const{
  return (next_propertyid - 1);
}
