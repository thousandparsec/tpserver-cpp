#ifndef SPLITFLEET_H
#define SPLITFLEET_H
/*  SplitFleet order
 *
 *  Copyright (C) 2004  Lee Begg and the Thousand Parsec Project
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

#include "order.h"
#include <map>

class SplitFleet : public Order{
 public:
  SplitFleet();
  virtual ~SplitFleet();

  void createFrame(Frame * f, int objID, int pos);
  bool inputFrame(Frame * f, unsigned int playerid);
  
  bool doOrder(IGObject * ob);
  
    std::map<uint32_t, uint32_t> getShips() const;
    void addShips(uint32_t designid, uint32_t count);

  void describeOrder(Frame * f) const;
  Order* clone() const;
  
 private:
  std::map<uint32_t, uint32_t> ships;

};

#endif
