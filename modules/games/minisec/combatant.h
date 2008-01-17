#ifndef COMBATANT_H
#define COMBATANT_H
/*  Combatant class
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <list>
#include <stdint.h>

#include <tpserver/vector3d.h>


class Combatant{
  public:
    Combatant();
    ~Combatant();

    uint32_t getOwner() const;
    void setOwner(uint32_t no);

    void addShips(uint32_t type, uint32_t number);
    bool removeShips(uint32_t type, uint32_t number);
    uint32_t numShips(uint32_t type);
    std::map<uint32_t, uint32_t> getShips() const;
    uint32_t totalShips() const;

    std::list<uint32_t> firepower(bool draw);
    bool hit(std::list<uint32_t> firepower);

    uint32_t getDamage() const;
    void setDamage(uint32_t nd);

  private:
    uint32_t playerref;
    std::map<uint32_t, uint32_t> shiplist;
    uint32_t damage;

};

#endif
