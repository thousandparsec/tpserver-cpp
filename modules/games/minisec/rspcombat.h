#ifndef RSPCOMBAT_H
#define RSPCOMBAT_H
/*  Rock Scissors Paper combat strategy
 *
 *  Copyright (C) 2004, 2007  Lee Begg and the Thousand Parsec Project
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
#include <set>
#include <vector>
#include <stdint.h>

class IGObject;
class Combatant;

namespace BattleXML {
    class BattleLogger;
}

class RSPCombat{
 public:
  RSPCombat();
  ~RSPCombat();

  void doCombat(std::map<uint32_t, std::set<uint32_t> > sides);


 private:
  std::map<Combatant*, uint32_t> buildShotList(std::vector<Combatant*> combatants, bool isDraw = false);
  void doDamage(std::map<Combatant*, uint32_t> shotlist, std::vector<Combatant*> targets);
  bool isAllDead(std::vector<Combatant*> combatants);
  void resolveCombatantsToObjects(std::vector<Combatant*> combatants);
   
  uint32_t obT_Fleet;
  uint32_t obT_Planet;
  std::map<uint32_t, IGObject*> objectcache;
  std::map<uint32_t, std::string> msgstrings;
  BattleXML::BattleLogger* battlelogger;
};

#endif
