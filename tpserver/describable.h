#ifndef DESCRIBABLE_H
#define DESCRIBABLE_H

/*  Describable trait class
 *
 *  Copyright (C) 2009 Kornel Kisielewicz and the Thousand Parsec Project
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

#include <tpserver/nameable.h>

class Describable : public Nameable {
  public:
    Describable( uint32_t new_id, const std::string& new_name = "", const std::string& new_desc = "" ) : Nameable( new_id, new_name ), desc( new_desc ) {}
    const std::string& getDescription() const { return desc; }
    void setDescription( const std::string& new_desc ) { desc = new_desc; }
  private:
    std::string desc;
};

#endif // DESCRIBABLE_H

