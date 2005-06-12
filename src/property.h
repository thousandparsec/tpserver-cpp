#ifndef PROPERTY_H
#define PROPERTY_H
/*  Component/Design Property class
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

class Property{
 public:
  Property();
  ~Property();

  void packFrame(Frame* frame) const;

  unsigned int getPropertyId() const;
  unsigned int getCategoryId() const;
  unsigned int getRank() const;
  std::string getName() const;
  std::string getTpclDisplayFunction() const;
  
  void setPropertyId(unsigned int id);
  void setCategoryId(unsigned int id);
  void setRank(unsigned int r);
  void setName(const std::string& n);
  void setDescription(const std::string& d);
  void setTpclDisplayFunction(const std::string& d);

 private:
  unsigned int propid;
  unsigned int catid;
  unsigned int rank;
  std::string name;
  std::string description;
  std::string tpcl_display;

};

#endif
