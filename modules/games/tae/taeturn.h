#ifndef TAETURN_H
#define TAETURN_H
/*  TaeTurn class, the end of turn process for tae
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
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

#include <tpserver/turnprocess.h>
#include "fleetbuilder.h"

class TaeTurn : public TurnProcess{
  public:
    TaeTurn(FleetBuilder* fb);
    virtual ~TaeTurn();
    
    virtual void doTurn();
  
    void setFleetType(uint32_t ft);
    void setPlanetType(uint32_t pt);
    
    std::set<uint32_t> getContainerIds() const;
    
  private:
    FleetBuilder* fleetBuilder;
    int playerTurn;
    uint32_t planettype;
    uint32_t fleettype;
    std::set<uint32_t> containerids;
};

#endif
