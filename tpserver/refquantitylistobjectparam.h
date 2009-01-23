#ifndef REFQUANTITYLISTOBJECTPARAM_H
#define REFQUANTITYLISTOBJECTPARAM_H
/*  RefQuantityListObjectParam class
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

class RefQuantityListObjectParam : public ObjectParameter{

public:
  RefQuantityListObjectParam();
  virtual ~RefQuantityListObjectParam();

  virtual void packObjectFrame(Frame * f, uint32_t objID);
  virtual bool unpackModifyObjectFrame(Frame * f, uint32_t playerid);

  virtual ObjectParameter *clone() const;

  typedef std::pair<int32_t, uint32_t> RefTypeAndId;
  typedef std::map<RefTypeAndId, uint32_t> RefQuanitityList;
  
  RefQuanitityList getRefQuantityList() const;
  void setRefQuantityList(RefQuanitityList nt);

protected:
	RefQuanitityList refquant;

};

#endif
