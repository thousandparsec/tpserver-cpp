/*  Resource Manager
 *
 *  Copyright (C) 2006  Lee Begg and the Thousand Parsec Project
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
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <tpserver/common.h>
#include <tpserver/resourcedescription.h>

class ResourceDescription;

/**
	@author Lee Begg <llnz@paradise.net.nz>
*/
class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    void init();
    
    uint32_t addResourceDescription(ResourceDescription::Ptr res);
    
    const ResourceDescription::Ptr getResourceDescription(uint32_t restype);
    const ResourceDescription::Ptr getResourceDescription(const std::string& restype);
    
    IdSet getAllIds();

private:
    typedef std::map<uint32_t, ResourceDescription::Ptr> ResourceMap;
    ResourceMap resdescs;
    uint32_t nextid;

};

#endif
