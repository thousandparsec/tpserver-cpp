#ifndef REFERENCEOBJECTPARAM_H
#define REFERENCEOBJECTPARAM_H
/*  ReferenceObjectParam class
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

#include <tpserver/objectparameter.h>

class ReferenceObjectParam : public ObjectParameter{

public:
  ReferenceObjectParam();
  virtual ~ReferenceObjectParam();

  virtual void packObjectFrame(Frame * f, uint32_t objID);
  virtual bool unpackModifyObjectFrame(InputFrame * f, uint32_t playerid);

  virtual ObjectParameter *clone() const;
  
  int32_t getReferenceType() const;
  void setReferenceType(int32_t nt);
  
  uint32_t getReferencedId() const;
  void setReferencedId(uint32_t ni);

protected:
  int32_t reftype;
  uint32_t refid;

};

#endif
