#ifndef CATEGORY_H
#define CATEGORY_H
/*  Category class
 *
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include <stdint.h>

class Frame;
class Design;

class Category{
 public:
  Category();
  virtual ~Category();

  uint32_t getCategoryId() const;
  std::string getName() const;
  std::string getDescription() const;
  uint64_t getModTime() const;
  void packFrame(Frame* frame) const;

  virtual bool doAddDesign(Design* d);
  virtual bool doModifyDesign(Design* d);

  void setCategoryId(uint32_t c);
  void setName(const std::string& n);
  void setDescription(const std::string& d);
  void setModTime(uint64_t nmt);

 protected:
  uint32_t catid;
  std::string name;
  std::string desc;
  uint64_t modtime;

};

#endif
