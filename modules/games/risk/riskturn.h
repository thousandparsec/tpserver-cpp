#ifndef RISKTURN_H
#define RISKTURN_H
/*  RiskTurn class, the end of turn process for risk
 * 
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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
#include <set>
 
#include <tpserver/turnprocess.h>
namespace RiskRuleset{

class RiskTurn : public TurnProcess{
public:
   RiskTurn();
   virtual ~RiskTurn();

   virtual void doTurn();
   void setPlayerVisibleObjects();
   
   Player* getWinner();
private:
   void processOrdersOfGivenType(std::string type = "");
   void calculateReinforcements();

};//class RiskTurn : public TurnProcess
} //namespace RiskRuleset
#endif
