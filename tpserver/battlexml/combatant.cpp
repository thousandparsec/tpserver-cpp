/*  Combatant class
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
 
#include "combatant.h"

namespace BattleXML {
    
    Combatant::Combatant(){
    }
    
    Combatant::~Combatant(){
    }
    
    std::string Combatant::getBattleXmlId() const{
        return battlexmlid;
    }
    
    void Combatant::setBattleXmlId(const std::string& id){
        battlexmlid = id;
    }
    
    std::string Combatant::getBattleXmlName() const{
        return battlexmlname;
    }
    
    void Combatant::setBattleXmlName(const std::string& name){
        battlexmlname = name;
    }
    
    std::string Combatant::getBattleXmlType() const{
        return battlexmltype;
    }
    
    void Combatant::setBattleXmlType(const std::string& type){
        battlexmltype = type;
    }
    
}
