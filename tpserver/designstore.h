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

  Category* getCategory(unsigned int id);
  Design* getDesign(unsigned int id);
  Component* getComponent(unsigned int id);
  Property* getProperty(unsigned int id);

  std::set<unsigned int> getCategoryIds() const;
  std::set<unsigned int> getDesignIds() const;
  std::set<unsigned int> getComponentIds() const;
  std::set<unsigned int> getPropertyIds() const;
 
  void addCategory(Category* c);

  bool addDesign(Design* d);
  bool modifyDesign(Design* d);
    void designCountsUpdated(Design* d);

  void addComponent(Component* c);
  void addProperty(Property* p);
  
  unsigned int getCategoryByName(const std::string& name);
  unsigned int getPropertyByName(const std::string& name);

  unsigned int getMaxDesignId() const;
  unsigned int getMaxComponentId() const;
  unsigned int getMaxPropertyId() const;
  
 protected:
  uint32_t next_designid;
  uint32_t next_componentid;
  uint32_t next_propertyid;
  uint32_t next_categoryid;
  std::map<unsigned int, Category*> categories;
  std::map<std::string,unsigned int>  categoryIndex;
  std::map<unsigned int, Design*> designs;
  std::map<unsigned int, Component*> components;
  std::map<unsigned int, Property*> properties;
  std::map<std::string,unsigned int>  propertyIndex;
  
};

#endif
