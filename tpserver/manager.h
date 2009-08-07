#ifndef MANAGER_H
#define MANAGER_H
/* Manager class
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

#include <tpserver/common.h>
#include <boost/shared_ptr.hpp>

template< class ManagedObject >
class Manager {
  public:
    typedef std::map<uint32_t, boost::shared_ptr<ManagedObject> > Map;

    IdSet getAllIds(){
      IdSet all;
      typename Map::const_iterator it = map.begin();
      for(; it != map.end(); ++it){
        all.insert(it->first);
      }
      return all;
    }

    const uint32_t size() const {
      return map.size();
    }

  protected:
    void clear( const IdSet& idset ) {
      for(IdSet::iterator it = idset.begin(); it != idset.end(); ++it){
        map[*it] = boost::shared_ptr<ManagedObject>();
      }
    }

    Map map;
};

#endif // MANAGER_H
