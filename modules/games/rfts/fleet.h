#ifndef fleet_H
#define fleet_H
/*  fleet class
 *
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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
#include <utility>

#include "staticobject.h"
#include "ownedobject.h"
#include <tpserver/player.h>

namespace RFTS_ {

class Planet;

class FleetType : public StaticObjectType{
  public:
    FleetType();
    virtual ~FleetType();
  protected:
    ObjectBehaviour* createObjectBehaviour() const;
};

class Fleet : public StaticObject, public OwnedObject
{
 public:
   Fleet();
   virtual ~Fleet() {}

   uint32_t getOwner() const;
   void setOwner(uint32_t no);

   int getDamage() const;
   void setDamage(int nd);
   void takeDamage(int dmg);

   const double getAttack() const;
   const double getSpeed() const;

   void setVelocity(const Vector3d& v);


   void recalcStats();
   
   void setOrderTypes(bool addColonise = false, bool addBombard = false);

   void addShips(uint32_t type, uint32_t number);
   bool removeShips(int type, uint32_t number);
   int numShips(int type);
   std::map<int,int> getShips() const;
   int totalShips() const;

   const bool isDead() const;
         
   virtual void packExtraData(OutputFrame::Ptr frame);   
   virtual void doOnceATurn();   
   virtual int getContainerType();
   
   void setupObject();

 private:

   double speed, attack, armour;
   bool hasTransports;

   bool setOpposingFleets( std::list<IGObject::Ptr >& fleets);
   bool doCombat(std::list<IGObject::Ptr >& fleets);
   void attackFleet(Fleet* opponent);
   void destroyShips(double intensity);

};

//helper functions
IGObject::Ptr createEmptyFleet(Player::Ptr player, IGObject::Ptr starSys, const std::string& name);
IGObject::Ptr createFleet(Player::Ptr player, IGObject::Ptr starSys, const std::string& name,
                      const IdMap& ships);
std::pair<IGObject::Ptr , bool> createFleet(Player::Ptr player, IGObject::Ptr starSys, const std::string& name,
                      const IdMap& ships, Planet *planetData);

}

#endif
