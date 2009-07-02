#ifndef PROTOCOLOBJECT_H
#define PROTOCOLOBJECT_H

/*  Protocol Object class
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

#include <tpserver/packable.h>
#include <tpserver/describable.h>
#include <tpserver/modifiable.h>

class ProtocolObject : public Packable, public Describable, public Modifiable {
public:
  ProtocolObject( uint32_t new_id, const std::string& new_name = "", const std::string& new_desc = "" ) : Describable( new_id, new_name, new_desc ) {}
};

#endif // PROTOCOLOBJECT_H
