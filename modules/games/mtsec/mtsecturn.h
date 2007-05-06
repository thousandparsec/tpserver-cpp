#ifndef MTSECTURN_H
#define MTSECTURN_H
/*  MTSecTurn class, the end of turn process for MTSec
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
#include <set>

#include <tpserver/turnprocess.h>

class MTSecTurn : public TurnProcess{
  public:
    MTSecTurn();
    virtual ~MTSecTurn();
    
    virtual void doTurn();
  
    void setFleetType(uint32_t ft);
    void setPlanetType(uint32_t pt);
    
    std::set<uint32_t> getContainerIds() const;
    
  private:
    uint32_t planettype;
    uint32_t fleettype;
    std::set<uint32_t> containerids;
};

#endif
