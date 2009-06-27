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

#include <boost/format.hpp>

#include "tinyxml/tinyxml.h"
#include "combatant.h"
#include "battleexceptions.h"

#include "battlelogger.h"

namespace BattleXML{
    BattleLogger::BattleLogger() : currside(NULL), currround(NULL), roundnum(0){
        bxmldoc = new TiXmlDocument();
        TiXmlElement* battle = new TiXmlElement("battle");
        battle->SetAttribute("version", "0.0.1");
        battle->SetAttribute("media", "minisec");
        bxmldoc->LinkEndChild(battle);
        
        sides = new TiXmlElement("sides");
        battle->LinkEndChild(sides);
        
        rounds = new TiXmlElement("rounds");
        battle->LinkEndChild(rounds);
    }
    
    BattleLogger::~BattleLogger(){
        delete bxmldoc;
        if(currside != NULL){
            delete currside;
        }
        if(currround != NULL){
            delete currround;
        }
    }
    
    void BattleLogger::startSide(const std::string& sidename){
        if(currside != NULL){
            throw SideInProgressException();
        }
        std::pair<std::set<std::string>::iterator, bool> insertpair = identifierset.insert(sidename);
        if(!insertpair.second){
            throw DuplicateIdentifierException(sidename);
        }
        currside = new TiXmlElement("side");
        currside->SetAttribute("id", sidename);
    }
    
    void BattleLogger::endSide(){
        if(currside == NULL){
            throw NoCurrentSideException();
        }
        sides->LinkEndChild(currside);
        currside = NULL;
    }
    
    void BattleLogger::addCombatant(Combatant* combatant){
        if(currside == NULL){
            throw NoCurrentSideException();
        }
        std::pair<std::set<std::string>::iterator, bool> insertpair =
                identifierset.insert(combatant->getBattleXmlId());
        if(!insertpair.second){
            throw DuplicateIdentifierException(combatant->getBattleXmlId());
        }
        TiXmlElement* entity = new TiXmlElement("entity");
        entity->SetAttribute("id", combatant->getBattleXmlId());
        TiXmlElement* name = new TiXmlElement("name");
        name->LinkEndChild(new TiXmlText(combatant->getBattleXmlName()));
        entity->LinkEndChild(name);
        TiXmlElement* type = new TiXmlElement("type");
        type->LinkEndChild(new TiXmlText(combatant->getBattleXmlType()));
        entity->LinkEndChild(type);
        currside->LinkEndChild(entity);
    }
    
    void BattleLogger::startRound(){
        if(currround != NULL){
            throw RoundInProgressException();
        }
        currround = new TiXmlElement("round");
        currround->SetAttribute("roundnum", roundnum);
    }
    
    void BattleLogger::endRound(){
        if(currround == NULL){
            throw NoCurrentRoundException();
        }
        rounds->LinkEndChild(currround);
        currround = NULL;
        roundnum++;
    }
    
    void BattleLogger::log(const std::string& logentry){
        if(currround == NULL){
            throw NoCurrentRoundException();
        }
        TiXmlElement* elog = new TiXmlElement("log");
        TiXmlText* elogtext = new TiXmlText(logentry);
        elog->LinkEndChild(elogtext);
        currround->LinkEndChild(elog);
    }
    
    void BattleLogger::move(Combatant* combatant, Vector3d pos){
        if(currround == NULL){
            throw NoCurrentRoundException();
        }
        if(identifierset.count(combatant->getBattleXmlId()) != 1){
            throw UnknownIdentifierException(combatant->getBattleXmlId());
        }
        TiXmlElement* emove = new TiXmlElement("move");
        TiXmlElement* ref = new TiXmlElement("reference");
        ref->SetAttribute("ref", combatant->getBattleXmlId());
        emove->LinkEndChild(ref);
        TiXmlElement* dest = new TiXmlElement("position");
        std::string posstr = str(boost::format("%1%,%2%,%3%") % pos.getX() % pos.getY() % pos.getZ());
        dest->LinkEndChild(new TiXmlText(posstr));
        emove->LinkEndChild(dest);
        currround->LinkEndChild(emove);
    }
    
    void BattleLogger::fire(Combatant* shooter, Combatant* target){
        if(currround == NULL){
            throw NoCurrentRoundException();
        }
        if(identifierset.count(shooter->getBattleXmlId()) != 1){
            throw UnknownIdentifierException(shooter->getBattleXmlId());
        }
        if(identifierset.count(target->getBattleXmlId()) != 1){
            throw UnknownIdentifierException(target->getBattleXmlId());
        }
        TiXmlElement* efire = new TiXmlElement("damage");
        TiXmlElement* ref = new TiXmlElement("source");
        ref->SetAttribute("ref", shooter->getBattleXmlId());
        efire->LinkEndChild(ref);
        ref = new TiXmlElement("destination");
        ref->SetAttribute("ref", target->getBattleXmlId());
        efire->LinkEndChild(ref);
        currround->LinkEndChild(efire);
    }
    
    void BattleLogger::damage(Combatant* combatant, uint32_t firepower){
        if(currround == NULL){
            throw NoCurrentRoundException();
        }
        if(identifierset.count(combatant->getBattleXmlId()) != 1){
            throw UnknownIdentifierException(combatant->getBattleXmlId());
        }
        TiXmlElement* edamage = new TiXmlElement("damage");
        TiXmlElement* ref = new TiXmlElement("reference");
        ref->SetAttribute("ref", combatant->getBattleXmlId());
        edamage->LinkEndChild(ref);
        TiXmlElement* amount = new TiXmlElement("amount");
        amount->LinkEndChild(new TiXmlText(str(boost::format("%1%") % firepower)));
        edamage->LinkEndChild(amount);
        currround->LinkEndChild(edamage);
    }
    
    void BattleLogger::death(Combatant* combatant){
        if(currround == NULL){
            throw NoCurrentRoundException();
        }
        if(identifierset.count(combatant->getBattleXmlId()) != 1){
            throw UnknownIdentifierException(combatant->getBattleXmlId());
        }
        TiXmlElement* edeath = new TiXmlElement("death");
        TiXmlElement* ref = new TiXmlElement("reference");
        ref->SetAttribute("ref", combatant->getBattleXmlId());
        edeath->LinkEndChild(ref);
        currround->LinkEndChild(edeath);
    }
}
