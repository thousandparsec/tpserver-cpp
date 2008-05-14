/*  Risk rulesset class
 *
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
 
// System includes 
#include <sstream> 

// tpserver includes 
#include <tpserver/game.h> 
#include <tpserver/logging.h> 
#include <tpserver/objectdatamanager.h> 
#include <tpserver/ordermanager.h> 
#include <tpserver/player.h> 

/* Minisec Includes
#include <tpserver/game.h>
#include <tpserver/object.h>
#include <tpserver/objectview.h>
#include <tpserver/objectmanager.h>
#include "universe.h"
#include "emptyobject.h"
#include "planet.h"
#include "fleet.h"
#include <tpserver/objecttypemanager.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include "rspcombat.h"
#include <tpserver/designstore.h>
#include <tpserver/ordermanager.h>
#include "nop.h"
#include "move.h"
#include "build.h"
#include "colonise.h"
#include "splitfleet.h"
#include "mergefleet.h"
#include <tpserver/property.h>
#include <tpserver/component.h>
#include <tpserver/design.h>
#include <tpserver/designview.h>
#include <tpserver/category.h>
#include <tpserver/logging.h>
#include <tpserver/playermanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/settings.h>
#include <tpserver/prng.h>
#include <tpserver/playermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include "minisecturn.h"
*/

// risk includes 

// header include 
#include "risk.h" 

//the libtool magic required
extern "C" { 
    #define tp_init librisk_LTX_tp_init 
    bool tp_init(){ 
        return Game::getGame()->setRuleset(new Risk::MyRisk());
    } 
}

Risk::Risk(){
	
}

Risk::~Risk(){
    
}

std::string Risk::getName(){
    return "Risk";
}

std::string Risk::getVersion(){
    return "0.1";
}

void Risk::initGame(){
    Logger::getLogger()->info("Risk initialised");
}

void Risk::createGame(){
    Logger::getLogger()->info("Risk created");
}

void Risk::startGame(){
    Logger::getLogger()->info("Risk started");
}

bool Risk::onAddPlayer(Player* player){
    Logger::getLogger()->debug("Risk onAddPlayer"); 
    return true; 
}

void Risk::onPlayerAdded(Player* player){
    Logger::getLogger()->debug("Risk onPlayerAdded");
}
