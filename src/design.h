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
#include <list>
#include <map>

#include "propertyvalue.h"

class Frame;

class Design{
 public:
  Design();
  virtual ~Design();

  void packFrame(Frame* frame) const;

  unsigned int getDesignId() const;
  unsigned int getOwner() const;
  unsigned int getNumExist() const;
  bool isValid() const;

  void setDesignId(unsigned int id);

  void eval();

 protected:
  unsigned int designid;
  unsigned int catid;
  std::string name;
  std::string description;
  unsigned int inuse;
  unsigned int exist;
  unsigned int owner;
  bool valid;
  std::list<unsigned int> components;
  std::map<unsigned int, PropertyValue> properties;
  std::string feedback;

};

#endif
