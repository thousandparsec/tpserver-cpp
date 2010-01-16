#ifndef BATTLEXML_BATTLEEXCEPTIONS_H
#define BATTLEXML_BATTLEEXCEPTIONS_H
/*  BattleExceptions, DuplicateIdentifier, UnknownIdentifier, 
 *    NoCurrentSide, NoCurrentRound, SideInProgress, RoundInProgress
 *    exception classes
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

#include <exception>
#include <string>

namespace BattleXML{
    
    class BattleException : public std::exception{
        public:
            BattleException();
            virtual const char* what() const throw();
    };
    
    class UnknownIdentifierException : public BattleException{
        public:
            UnknownIdentifierException(const std::string& id);
            virtual ~UnknownIdentifierException() throw ();
            virtual const char* what() const throw();
            
        private:
            std::string identifier;
    };
    
    class DuplicateIdentifierException : public BattleException{
        public:
            DuplicateIdentifierException(const std::string& id);
            virtual ~DuplicateIdentifierException() throw ();
            virtual const char* what() const throw();
            
        private:
            std::string identifier;
    };
    
    class NoCurrentSideException : public BattleException{
        public:
            NoCurrentSideException();
            virtual const char* what() const throw();
    };
    
    class NoCurrentRoundException : public BattleException{
        public:
            NoCurrentRoundException();
            virtual const char* what() const throw();
    };
    
    class SideInProgressException : public BattleException{
        public:
            SideInProgressException();
            virtual const char* what() const throw();
    };
    
    class RoundInProgressException : public BattleException{
        public:
            RoundInProgressException();
            virtual const char* what() const throw();
    };
    
    class FailedToSaveException : public BattleException{
        public:
            FailedToSaveException();
            virtual const char* what() const throw();
    };

}
#endif
