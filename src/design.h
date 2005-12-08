#ifndef DESIGN_H
#define DESIGN_H
/*  Design class
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

#include <string>
#include <map>

#include "propertyvalue.h"

class Frame;

class Design{
 public:
  Design();
  virtual ~Design();

  void packFrame(Frame* frame) const;

  unsigned int getDesignId() const;
  unsigned int getCategoryId() const;
  std::string getName() const;
    std::string getDescription() const;
  unsigned int getOwner() const;
  std::map<unsigned int, unsigned int> getComponents() const;
  unsigned int getNumExist() const;
    uint32_t getInUse() const;
  bool isValid() const;
    std::string getFeedback() const;
  double getPropertyValue(unsigned int propid) const;
    std::map<uint32_t, PropertyValue> getPropertyValues() const;
    uint64_t getModTime() const;

  void setDesignId(unsigned int id);
  void setCategoryId(unsigned int id);
  void setName(const std::string& n);
  void setDescription(const std::string& d);
  void setOwner(unsigned int o);
  void setComponents(std::map<unsigned int, unsigned int> cl);
    void setInUse(uint32_t niu);
    void setNumExist(uint32_t nne);
    void setModTime(uint64_t nmt);

  void eval();
  void setPropertyValues(std::map<unsigned int, PropertyValue> pvl);
  void setValid(bool v, const std::string& f);

 protected:
  unsigned int designid;
  unsigned int catid;
  std::string name;
  std::string description;
  unsigned int inuse;
  unsigned int exist;
  unsigned int owner;
  bool valid;
  unsigned long long timestamp;
  std::map<unsigned int, unsigned int> components;
  std::map<unsigned int, PropertyValue> properties;
  std::string feedback;

};

#endif
