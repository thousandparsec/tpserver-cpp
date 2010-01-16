#ifndef BATTLEXML_BATTLELOGGER_H
#define BATTLEXML_BATTLELOGGER_H
/*  BattleLogger class
 *
 *  Copyright (C) 2009  Lee Begg and the Thousand Parsec Project
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
#include <string>
#include <set>
#include <boost/shared_ptr.hpp>

#include <tpserver/vector3d.h>

class TiXmlDocument;
class TiXmlElement;
class Combatant;

namespace BattleXML{
    
    class BattleLogger{
        public:
            typedef boost::shared_ptr<BattleLogger> Ptr;
            BattleLogger();
            ~BattleLogger();
            
            std::string save();
            
            void startSide(const std::string& sidename);
            void endSide();
            
            void addCombatant(Combatant* combatant);
            
            void startRound();
            void endRound();
            
            void log(const std::string& logentry);
            void move(Combatant* combatant, Vector3d pos);
            void fire(Combatant* shooter, Combatant* target);
            void damage(Combatant* combatant, uint32_t firepower);
            void death(Combatant* combatant);
            
        private:
            TiXmlDocument* bxmldoc;
            TiXmlElement* sides;
            TiXmlElement* rounds;
            TiXmlElement* currside;
            TiXmlElement* currround;
            
            uint32_t roundnum;
            std::set<std::string> identifierset;
    };
    
}

#endif
