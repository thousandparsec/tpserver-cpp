#ifndef SENDPOINTS_H
#define SENDPOINTS_H
/*  SendPoints order
 *
 *  Copyright (C) 2009 Alan P. Laudicina and the Thousand Parsec Project
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <map>
#include <string>

#include <tpserver/result.h>
#include <tpserver/order.h>
class ListParameter;
class TimeParameter;

namespace MTSecRuleset {

class SendPoints : public Order{
 public:
  SendPoints();
  virtual ~SendPoints();
  bool doOrder(IGObject *ob);
  Order* clone() const;
  std::map<uint32_t, std::pair<std::string, uint32_t> > generateListOptions();


 private:
  ListParameter* targetPlanet;
  uint32_t maxPoints;
  double getPercentage(IGObject *ob) const;
};

}
#endif
