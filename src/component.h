#ifndef COMPONENT_H
#define COMPONENT_H
/*  Design Component class
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

class Frame;

class Component{
 public:
  Component();
  virtual ~Component();

  void packFrame(Frame* frame) const;
  
 protected:
  unsigned int compid;
  unsigned int catid;
  std::string name;
  std::string description;
  std::string tpcl_add;
  std::map<unsigned int, std::string> propertylist;
  

};


#endif
