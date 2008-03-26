#ifndef OBJECTPARAMETERGROUP_H
#define OBJECTPARAMETERGROUP_H
/*  ObjectParameterGroup class
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

#include <stdint.h>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class Frame;
class ObjectParameter;
class ObjectParameterGroupData;

typedef boost::shared_ptr<ObjectParameterGroupData> ObjectParameterGroupPtr;

class ObjectParameterGroupData {

  public:
    ObjectParameterGroupData();
    ObjectParameterGroupData(const ObjectParameterGroupData& rhs);
    ~ObjectParameterGroupData();

    uint32_t getGroupId() const;

		typedef std::vector<ObjectParameter*> ParameterList;
    
    ParameterList getParameters() const;
    ObjectParameter* getParameter(uint32_t paramid) const;
    
    void setGroupId(uint32_t ni);
    
    void addParameter(ObjectParameter* op);
    
    void packObjectFrame(Frame * f, uint32_t playerid);
    bool unpackModifyObjectFrame(Frame * f, unsigned int playerid);
    
    void signalRemoval();

  protected:
      uint32_t groupid;
		ParameterList parameters;

};


#endif
