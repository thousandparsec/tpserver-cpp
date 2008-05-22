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
#include <tpserver/objectmanager.h> 
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
        return Game::getGame()->setRuleset(new RiskRuleset::Risk());
    } 
}

namespace RiskRuleset {

Risk::Risk(){
//Minisec has a parent of random(NULL), whats with that?	
}

Risk::~Risk(){
//minisec simply deletes the random if not NULL    
}

std::string Risk::getName(){
    return "Risk";
}

std::string Risk::getVersion(){
    return "0.0";
}

void Risk::initGame(){
    Logger::getLogger()->info("Risk initialised");

    //Just modelling after minisec here, to give myself an idea of what goes on
    /*
    Game* game = Game::getGame(); <- Obviously required, a reference to the actual game
    
    MinisecTurn* turn = new MinisecTurn();
    game->setTurnProcess(turn) <-tells game to process turn using created reference
    
    SET OF CALLS RELATING TO ObjectTypeManager
    >minisec runs through creating empty object types, setting name/description, then adding to a
        >type manager
    >At the end he creates two uint32_t's that are newObjectTypes PlanetType() and FleetType()
        >These are then passed as arguments in setters for turn
        
    SET OF CALLS RELATING TO OrderManager
    >I'm seeing a set of OrderTypes added to an OrderManager, seems straight forward
    */
}

void Risk::createGame(){
    Logger::getLogger()->info("Risk created");
    //Modelling of minisec to give myself direction in development
    /*
    Get the game
    
    create catids set...
    
    a big set of properties being added about ships
    >Mine will most likely be very different
        >There won't be ships, armies won't have names, etc.
        >Territories will simply have an army size, thats it
        
    object and object type managers are grabbed from game
    
    object type names are grabbed from type manager
    
    a universe IGObject reference is created for the universe
    otypeman sets the object up with the IGO reference and the Type
    some more setup is done on the universe
    and it is added to the object manager
    
    A long series of objects are added INTO the universe, this includes some static 
        calls as well as random ones
    I want to go through how star systems and galaxies are created for myself:
    >
    >
    Its also notable that the method of getting settings from a conf is shown as:
      std::string thing  = Settings::getSettings()->get(SettingName)
     A number is grabbed like so:
      uint32_t min_systems = atoi(Settings::getSettings()->get("minisec_min_systems").c_str());
      
     Some resources are set up afterwards, and a little clean up of names used(not applicable to me)
    */
}

void Risk::startGame(){
    Logger::getLogger()->info("Risk started");
    //Modelling minisec
    /*
    There is some resource setup that is not applicable to risk
    followed by getting settings, this will be applicable and as such here is the snippet:
    Settings* settings = Settings::getSettings();
    if(settings->get("turn_length_over_threshold") == ""){
      settings->set("turn_length_over_threshold", "600");
      settings->set("turn_player_threshold", "0");
      settings->set("turn_length_under_threshold", "600");
      
     They look to be defaults if a turnlength isn't set
    */
}

bool Risk::onAddPlayer(Player* player){
    Logger::getLogger()->debug("Risk onAddPlayer"); 
    return true; 
}

void Risk::onPlayerAdded(Player* player){
    Logger::getLogger()->debug("Risk onPlayerAdded");
    
    //Modelling minisec
    /*
    Game reference is grabed
    
    playerview reference is grabbed
    
    if player is guest, set IsAlive to false, otherwise 
    
    set the player to see other designs (not sure what this does -- believe it relates to ships)
    Player gets some default objects created for them (NA to risk)
    (risk) -> should simply indicate the player gets some resources to spend 
        (ideally players won't join midgame thogh)
        
    create a system for player (NA to risk)
        
    */   
}
} //end namespace RiskRuleset