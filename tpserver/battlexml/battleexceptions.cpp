/*  BattleExceptions, DuplicateIdentifier, UnknownIdentifier, 
 *    NoCurrentSide, NoCurrentRound, SideInProgress, RoundInProgress
  *   exception classes
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

#include "battleexceptions.h"

namespace BattleXML{
    
    BattleException::BattleException(){
    }
    
    const char* BattleException::what() const throw(){
        return "Generic BattleException";
    }
    
    UnknownIdentifierException::UnknownIdentifierException(const std::string& id) :
            identifier(id){
    }
    
    UnknownIdentifierException::~UnknownIdentifierException() throw (){
    }
    
    const char* UnknownIdentifierException::what() const throw(){
        std::string msg = "Unknown identifier '" + identifier + "'";
        return msg.c_str();
    }
    
    DuplicateIdentifierException::DuplicateIdentifierException(const std::string& id) :
            identifier(id){
    }
    
    DuplicateIdentifierException::~DuplicateIdentifierException() throw (){
    }
    
    const char* DuplicateIdentifierException::what() const throw(){
        std::string msg = "Duplicate identifier '" + identifier + "'";
        return msg.c_str();
    }
    
    NoCurrentSideException::NoCurrentSideException(){
    }
    
    const char* NoCurrentSideException::what() const throw(){
        return "No current side started to be added to";
    }
    
    NoCurrentRoundException::NoCurrentRoundException(){
    }
    
    const char* NoCurrentRoundException::what() const throw(){
        return "No current round started to have events added to";
    }
    
    SideInProgressException::SideInProgressException(){
    }
    
    const char* SideInProgressException::what() const throw(){
        return "A side has already been started, not yet finished";
    }
    
    RoundInProgressException::RoundInProgressException(){
    }
    
    const char* RoundInProgressException::what() const throw(){
        return "A round had already been started, not yet finished";
    }
    
    FailedToSaveException::FailedToSaveException(){
    }
    
    const char* FailedToSaveException::what() const throw(){
        return "Failed to save the battle XML file";
    }
}
