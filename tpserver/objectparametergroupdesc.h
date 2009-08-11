#ifndef OBJECTPARAMETERGROUPDESC_H
#define OBJECTPARAMETERGROUPDESC_H
/*  ObjectParameterGroupDesc class
 * Collects Object Parameters together into a group.
 *
 *  Copyright (C) 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/describable.h>
#include <tpserver/packable.h>
#include <boost/tuple/tuple.hpp>

class Frame;
class ObjectParameterGroup;

class ObjectParameterGroupDesc : public Describable, public Packable {
  public:
    typedef boost::shared_ptr<ObjectParameterGroupDesc> Ptr;

    ObjectParameterGroupDesc( uint32_t nid, const std::string& pname, const std::string& pdesc );

    void addParameter(uint32_t ptype, const std::string& pname, const std::string& pdesc);

    void pack(OutputFrame* f) const;
    boost::shared_ptr<ObjectParameterGroup> createObjectParameterGroup() const;

  protected:
    typedef boost::tuple< uint32_t, std::string, std::string > ParameterDesc;
    typedef std::list<ParameterDesc> Parameters;
    Parameters parameters;

};


#endif
