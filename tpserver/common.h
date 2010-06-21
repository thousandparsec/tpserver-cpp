#ifndef COMMON_H
#define COMMON_H

/*  Common defines, typedefs and enums
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

#include <stdint.h>
#include <tpserver/protocol.h>
#include <boost/shared_ptr.hpp>
#include <string>
#include <set>
#include <map>
#include <list>

/// Typedef for Id set
typedef std::set< uint32_t >  IdSet;

/// Typedef for Id list
typedef std::list< uint32_t > IdList;

/// Typedef for map between Id's and Id's
typedef std::map< uint32_t, uint64_t > IdMap;

/// Typedef for map between Id's and modify times
typedef std::map< uint32_t, uint64_t > IdModList;

/// Typedef for map between Id's and strings
typedef std::map< uint32_t, std::string > IdStringMap;

/// Typedef for map between strings and Id's
typedef std::map< std::string, uint32_t > NameMap;

/// Typedef for Reference type and id
typedef std::pair<int32_t, uint32_t> RefTypeAndId;

/// Typedef for Reference list
typedef std::list<RefTypeAndId> RefList;

#endif // COMMON_H
