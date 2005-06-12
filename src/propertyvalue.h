#ifndef PROPERTYVALUE_H
#define PROPERTYVALUE_H
/*  The Value of a Property in a Design
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

class Frame;

class PropertyValue{
 public:
  PropertyValue();
  PropertyValue(const PropertyValue& rhs);
  ~PropertyValue();

  PropertyValue operator=(const PropertyValue& rhs);
  bool operator==(const PropertyValue& rhs) const;
  bool operator<(const PropertyValue& rhs) const;

  void packFrame(Frame* frame) const;

  unsigned int getPropertyId() const;
  double getValue() const;
  std::string getDisplayString() const;

  void setPropertyId(unsigned int id);
  void setValue(double v);
  void setDisplayString(const std::string& d);

 private:
  unsigned int propid;
  double value;
  std::string display;

};

#endif
