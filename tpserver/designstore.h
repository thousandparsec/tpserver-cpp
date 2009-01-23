#ifndef DESIGNSTORE_H
#define DESIGNSTORE_H
/*  Design/Component/Property storage class
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

#include <set>
#include <map>
#include <string>

class Category;
class Design;
class Component;
class Property;

class DesignStore{
 public:
  DesignStore();
  ~DesignStore();

    void init();

  Category* getCategory(uint32_t id);
  Design* getDesign(uint32_t id);
  Component* getComponent(uint32_t id);
  Property* getProperty(uint32_t id);

  std::set<uint32_t> getCategoryIds() const;
  std::set<uint32_t> getDesignIds() const;
  std::set<uint32_t> getComponentIds() const;
  std::set<uint32_t> getPropertyIds() const;
 
  void addCategory(Category* c);

  bool addDesign(Design* d);
  bool modifyDesign(Design* d);
    void designCountsUpdated(Design* d);

  void addComponent(Component* c);
  void addProperty(Property* p);
  
  uint32_t getCategoryByName(const std::string& name);
  uint32_t getComponentByName(const std::string& name);
  uint32_t getPropertyByName(const std::string& name);

  uint32_t getMaxDesignId() const;
  uint32_t getMaxComponentId() const;
  uint32_t getMaxPropertyId() const;
  
 protected:
  uint32_t next_designid;
  uint32_t next_componentid;
  uint32_t next_propertyid;
  uint32_t next_categoryid;
  std::map<uint32_t, Category*> categories;
  std::map<std::string,uint32_t>  categoryIndex;
  std::map<uint32_t, Design*> designs;
  std::map<uint32_t, Component*> components;
  std::map<std::string, uint32_t> componentIndex;
  std::map<uint32_t, Property*> properties;
  std::map<std::string,uint32_t>  propertyIndex;
  
};

#endif
