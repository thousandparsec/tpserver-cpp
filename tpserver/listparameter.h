#ifndef LISTPARAMETER_H
#define LISTPARAMETER_H
/*  ListParameter class
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

#include <tpserver/orderparameter.h>
#include <tpserver/common.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class ListParameter : public OrderParameter{

public:
  typedef std::pair<std::string, uint32_t> Option;
  typedef std::map< uint32_t, Option> Options;
  typedef boost::function< Options () > Callback;

  ListParameter(const std::string& aname, const std::string& adesc, Callback acallback);
  virtual ~ListParameter();

  virtual void packOrderFrame(Frame * f);
  virtual bool unpack(Frame * f);

  IdMap getList() const;
  void setList(IdMap nlist);
  
protected:
  IdMap list;
  Callback callback;
};

#endif
