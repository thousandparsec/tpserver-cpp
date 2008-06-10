/*  Graph class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project

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

#include <graph.h>
#include <risk.h>
#include <tpserver/object.h>
#include <tpserver/objectmanager.h>
#include <tpserver/game.h>
namespace RiskRuleset {

Graph::Graph() { 
   
}

Graph::~Graph() { 
   
}

bool Graph::addPlanet(IGObject* planet) { 
   bool result = false;
   
   Node node;
   node.id = planet->getID();
   nodeMap[node.id] = node;
   result = true;
   
   return result;
}

bool Graph::addEdge(uint32_t id1, uint32_t id2) { 
   bool result = false;
   //CHECK: what the default for the map is, and check that the id is declared
   Node* node1 = &nodeMap[id1];
   Node* node2 = &nodeMap[id2];
   if ( node1 != NULL && node2 != NULL ) { 
      node1->adjacent.insert(node2); 
      node2->adjacent.insert(node1);
      result = true; 
   }
   return result;
}

bool Graph::addEdge(IGObject* planet1, IGObject* planet2) { 
   return addEdge(planet1->getID(),planet2->getID());
}

std::set<uint32_t> Graph::getAdjacent(IGObject* planet) { 
   return getAdjacent(planet->getID());
}

std::set<uint32_t> Graph::getAdjacent(uint32_t id) { 
   std::set<uint32_t> result;
   Node* node = &nodeMap[id];
   for (std::set<Node*>::iterator i = node->adjacent.begin(); i != node->adjacent.end(); ++i) {
      result.insert((*i)->id);
   }
   return result;
}


} //end namespace RiskRuleset
