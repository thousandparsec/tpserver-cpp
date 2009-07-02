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
#include <tpserver/modifiable.h>
#include <tpserver/describable.h>
#include <tpserver/packable.h>

class Frame;
class Design;

class Category : public Modifiable, public Describable, public Packable {
  public:
    Category();
    virtual ~Category();

    /// TODO: Remove, use getId instead
    uint32_t getCategoryId() const;
    void pack(Frame* frame) const;

    virtual bool doAddDesign(Design* d);
    virtual bool doModifyDesign(Design* d);

    /// TODO: Remove, use setId instead
    void setCategoryId(uint32_t c);
};

#endif
