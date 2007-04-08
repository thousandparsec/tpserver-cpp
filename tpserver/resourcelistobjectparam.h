#ifndef RESOURCELISTOBJECTPARAM_H
#define RESOURCELISTOBJECTPARAM_H
/*  ResourceListObjectParam class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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
#include <map>

#include <tpserver/objectparameter.h>

class ResourceListObjectParam : public ObjectParameter{

public:
  ResourceListObjectParam();
  virtual ~ResourceListObjectParam();

  virtual void packObjectFrame(Frame * f, uint32_t objID);
  virtual bool unpackModifyObjectFrame(Frame * f, unsigned int playerid);

  virtual ObjectParameter *clone() const;
  
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > getResources() const;
  void setResources(std::map<uint32_t, std::pair<uint32_t, uint32_t> > nt);

protected:
  std::map<uint32_t, std::pair<uint32_t, uint32_t> > resources;

};

#endif
