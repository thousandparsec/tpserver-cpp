#ifndef RISK_H
#define RISK_H
/*  Risk rulesset class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2005, 2007  Lee Begg and the Thousand Parsec Project
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

//small change
#include <tpserver/ruleset.h> 
#include <boost/graph/adjacency_matrix.hpp>

class IGObject;

namespace RiskRuleset {
	
class Risk : public Ruleset {
   public:
      Risk();
      virtual ~Risk();

      std::string getName(); 
      std::string getVersion(); 
      void initGame(); 
      void createGame(); 
      void startGame(); 
      bool onAddPlayer(Player* player); 
      void onPlayerAdded(Player* player);
   private:
       //initGame methods
      void setObjectTypes() const;
      void setOrderTypes() const;

       //createGame methods
      void createResources();

      void createUniverse();
      IGObject* createGalaxy(IGObject& parent, const std::string& name, int bonus, uint32_t id);
      IGObject* createStarSystem(IGObject& parent, const std::string& name,
         double unitX, double unitY, uint32_t id);
      IGObject* createPlanet(IGObject& parent, const std::string& name,
         double unitX, double unitY, uint32_t id);                        
      IGObject* createPlanet(IGObject& parentStarSys, const std::string& name,
         const Vector3d& location, uint32_t id);

       //onAddPlayer methods
      bool isBoardClaimed() const;
      
      //The number of planets to be on the board
      int num_planets;
      
      //TODO: Change to adacency_list: The graph of planets is more than likely far to sparse to warrant a matrix
      typedef boost::adjacency_matrix<boost::undirectedS> UGraph; 
      UGraph matrix;

};// class Risk : public Ruleset
	
} // namespace RiskRuleset
#endif
