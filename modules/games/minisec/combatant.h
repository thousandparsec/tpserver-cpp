#ifndef COMBATANT_H
#define COMBATANT_H
/*  Combatant class
 *
 *  Copyright (C) 2004-2005, 2007, 2008, 2009  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/common.h>
#include <tpserver/vector3d.h>
#include <tpserver/battlexml/combatant.h>


class Combatant : public BattleXML::Combatant{
  public:
    Combatant();
    ~Combatant();

    playerid_t getOwner() const;
    void setOwner(playerid_t no);
    
    objectid_t getObject() const;
    void setObject(objectid_t no);

    void setShipType(designid_t type);
    designid_t getShipType() const;

    uint32_t firepower(bool draw);
    bool hit(uint32_t firepower);

    uint32_t getDamage() const;
    void setDamage(uint32_t nd);
    
    bool isDead() const;

  private:
    playerid_t playerref;
    objectid_t objectref;
    designid_t shiptype;
    uint32_t damage;
    IdMap shiplist;

};

#endif
