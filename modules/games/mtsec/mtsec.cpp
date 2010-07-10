/*  MTSec ruleset
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <cassert>
#include <stdlib.h>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tpserver/game.h"
#include "tpserver/object.h"
#include <tpserver/objectview.h>
#include "tpserver/objectmanager.h"
#include "universe.h"
#include "emptyobject.h"
#include "planet.h"
#include "fleet.h"
#include "tpserver/objecttypemanager.h"
#include "tpserver/player.h"
#include <tpserver/playerview.h>
#include "avacombat.h"
#include "tpserver/ordermanager.h"
#include "nop.h"
#include "move.h"
#include "buildfleet.h"
#include "buildweapon.h"
#include "enhance.h"
#include "colonise.h"
#include "splitfleet.h"
#include "mergefleet.h"
#include "sendpoints.h"
#include "loadarm.h"
#include "unloadarm.h"
#include "tpserver/property.h"
#include "tpserver/component.h"
#include "tpserver/design.h"
#include "tpserver/category.h"
#include "tpserver/logging.h"
#include "tpserver/playermanager.h"
#include "tpserver/prng.h"
#include <tpserver/resourcedescription.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/settings.h>
#include <tpserver/net.h>
#include "mtsecturn.h"
#include "xmlimport.h"

#include "mtsec.h"

namespace MTSecRuleset {

static char const * const systemNames[] = {
    "Barnard's Star",  "Gielgud",             "Ventana",
    "Aleph Prime",     "Ventil",              "Sagitaria",
    "Drifter",         "Ptelemicus",          "Centanis",
    "Mendelis",        "Cassious' Shadow",    "Llentim",
    "Redoubt",         "Kelper",              "Cemara",
    "Cilantarius",     "Kya",                 "Lanternis",
    "Illatis",         "Rintim",              "Uvaharim",
    "Plaetais",        "Denderis",            "Desiderata",
    "Illuntara",       "Ivemteris",           "Wetcher",
    "Monanara",        "Clesasia",            "RumRunner",
    "Last Chance",     "Kiuper Shadow",       "NGC 42059",
    "Ceti Alpha",      "Surreptitious",       "Lupus Fold",
    "Atlantis",        "Draconis",            "Muir's Gold",
    "Fools Errand",    "Wrenganis",           "Humph",
    "Byzantis",        "Torontis",            "Radiant Pool"};


extern "C" {
  #define tp_init libmtsec_LTX_tp_init
  bool tp_init(){
    return Game::getGame()->setRuleset(new MTSecRuleset::MTSec());
  }
}

MTSec::MTSec() {
  Settings* settings = Settings::getSettings();
  xmlImporter = new xmlImport();
  xmlImporter->setFile(settings->get("gamedata_file"));
}


MTSec::~MTSec() {

}


std::string MTSec::getName(){
  return "MTSec";
}

std::string MTSec::getVersion(){
  return "0.3";
}

void MTSec::initGame() {
    Game* game = Game::getGame();

    MTSecTurn* turn = new MTSecTurn();
    
    game->setTurnProcess(turn);

    ObjectTypeManager* obdm = game->getObjectTypeManager();
    obdm->addNewObjectType( new UniverseType());
    obdm->addNewObjectType( new EmptyObjectType( "Galaxy", "The Galaxy Object type") );
    obdm->addNewObjectType( new EmptyObjectType( "Star System","The Star System Object type") );
    uint32_t pt = obdm->addNewObjectType( new PlanetType());
    uint32_t ft = obdm->addNewObjectType( new FleetType());
    
    turn->setPlanetType(pt);
    turn->setFleetType(ft);

    OrderManager* ordm = game->getOrderManager();
    ordm->addOrderType(new Nop());
    ordm->addOrderType(new Move());
    ordm->addOrderType(new BuildFleet());
    ordm->addOrderType(new BuildWeapon());
    ordm->addOrderType(new Colonise());
    ordm->addOrderType(new SplitFleet());
    ordm->addOrderType(new MergeFleet());
    ordm->addOrderType(new Enhance());
    ordm->addOrderType(new SendPoints());
    ordm->addOrderType(new LoadArmament());
    ordm->addOrderType(new UnloadArmament());
}


#if 0
// Dependencies
"AmmoCost"          Ammo  0
"AmmoExplosiveness" Ammo  0
"AmmoSize"          Ammo  0
"Armor"             Hull  0
"BuildTime"         Hull  0
"Firepower"         Hull  2 => MissileSize, MissileFirepower
"HitPoints"         Hull  0
"MissileCost"       Bay   1 => AmmoSize, AmmoCost
"MissileFirepower"  Bay   1 => AmmoSize, AmmoExplosiveness
"MissileSize"       Bay   0
"Speed"             Hull  0
"StartingHitPoints" Hull  0
"num-ammo"          Ammo  0
"num-baytypes"      Bay   0
"num-hulls"         Hull  0
//    Needed by other things:
"Colonise"          Needed by Fleet::checkAllowedOrder();
#endif



void MTSec::createProperties()
{
    Logger::getLogger()->debug( "Enter MTSec::createProperties");

    assert(xmlImporter->importProps());
    Logger::getLogger()->debug("Done reading XML file.");


    Logger::getLogger()->debug( "Exit MTSec::createProperties");

    return;
}


void MTSec::createComponents()
{
    Logger::getLogger()->debug( "Enter MTSec::createComponents");

    assert(xmlImporter->importComps()); 
    Logger::getLogger()->debug("Done reading XML file.");

    Logger::getLogger()->debug( "Exit MTSec::createComponents");

    return;
}


void MTSec::createTechTree()
{
    Logger::getLogger()->debug( "Enter MTSec::createTechTree");

    createProperties();
    createComponents();

    Logger::getLogger()->debug( "Exit MTSec::createTechTree");

    return;
}


// Create the Alpha Centauri star system
IGObject::Ptr MTSec::createAlphaCentauriSystem( IGObject::Ptr mw_galaxy)
{
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    ResourceManager::Ptr resman = game->getResourceManager();
    IGObject::Ptr      ac = game->getObjectManager()->createNewObject();
    IGObject::Ptr      acprime = game->getObjectManager()->createNewObject();
    ObjectTypeManager* otypeman = game->getObjectTypeManager();

    uint32_t obT_Star_System = otypeman->getObjectTypeByName("Star System");
    uint32_t obT_Planet = otypeman->getObjectTypeByName("Planet");

    
    otypeman->setupObject(ac, obT_Star_System);
    EmptyObject* theac = dynamic_cast<EmptyObject*>(ac->getObjectBehaviour());
    theac->setSize(800000ll);
    ac->setName("Alpha Centauri System");
    theac->setPosition(Vector3d(-1500000000ll, 1500000000ll, 0ll));
    ac->addToParent(mw_galaxy->getID());
    obman->addObject(ac);

    otypeman->setupObject(acprime, obT_Planet);
    Planet* theacprime = dynamic_cast<Planet*>(acprime->getObjectBehaviour());
    theacprime->setSize(2);
    acprime->setName("Alpha Centauri Prime");
    theacprime->setPosition(theac->getPosition() + Vector3d(-6300ll, 78245ll, 0ll));
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ress[resman->getResourceDescription("Factories")->getResourceType()] = std::pair<uint32_t, uint32_t>(game->getRandom()->getInRange(1, 15), 0);
    theacprime->setResources(ress);
    
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(acprime->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(game->getOrderManager()->addOrderQueue(acprime->getID(), 0));
    theacprime->setDefaultOrderTypes();
    
    acprime->addToParent(ac->getID());
    obman->addObject(acprime);

    return ac;
}


// Create the Sirius star system
IGObject::Ptr MTSec::createSiriusSystem( IGObject::Ptr mw_galaxy)
{
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    ResourceManager::Ptr resman = game->getResourceManager();
    IGObject::Ptr      sirius = game->getObjectManager()->createNewObject();
    IGObject::Ptr      s1 = game->getObjectManager()->createNewObject();
    ObjectTypeManager* otypeman = game->getObjectTypeManager();
    
    uint32_t obT_Star_System = otypeman->getObjectTypeByName("Star System");
    uint32_t obT_Planet = otypeman->getObjectTypeByName("Planet");

    otypeman->setupObject(sirius, obT_Star_System);
    EmptyObject* thesirius = dynamic_cast<EmptyObject*>(sirius->getObjectBehaviour());
    thesirius->setSize(2000000ll);
    sirius->setName("Sirius System");
    thesirius->setPosition(Vector3d(-250000000ll, -3800000000ll, 0ll));
    sirius->addToParent(mw_galaxy->getID());
    obman->addObject(sirius);

    otypeman->setupObject(s1, obT_Planet);
    Planet* thes1 = dynamic_cast<Planet*>(s1->getObjectBehaviour());
    thes1->setSize(2);
    s1->setName("Sirius 1");
    thes1->setPosition(thesirius->getPosition() + Vector3d(45925ll, -34262ll, 0ll));
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ress[resman->getResourceDescription("Factories")->getResourceType()] = std::pair<uint32_t, uint32_t>(game->getRandom()->getInRange(1, 15), 0);
    thes1->setResources(ress);
    
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(s1->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId( game->getOrderManager()->addOrderQueue(s1->getID(),0) );
    thes1->setDefaultOrderTypes();
    
    s1->addToParent(sirius->getID());
    obman->addObject(s1);

    return sirius;
}


// Returns a random number between 1 and 'max'
static uint32_t myRandom( uint32_t  max)
{
    return Game::getGame()->getRandom()->getInRange(1U, max);
}


// Create a random star system
IGObject::Ptr MTSec::createStarSystem( IGObject::Ptr mw_galaxy)
{
    Logger::getLogger()->debug( "Entering MTSec::createStarSystem");
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    IGObject::Ptr      star = game->getObjectManager()->createNewObject();
    uint32_t   nplanets = 0;
    std::ostringstream     formatter;
    ObjectTypeManager* otypeman = game->getObjectTypeManager();
    
    uint32_t obT_Star_System = otypeman->getObjectTypeByName("Star System");
    uint32_t obT_Planet = otypeman->getObjectTypeByName("Planet");

    otypeman->setupObject(star, obT_Star_System);
    EmptyObject* thestar = dynamic_cast<EmptyObject*>(star->getObjectBehaviour());
    thestar->setSize(1400000ll);
    uint32_t   thx = myRandom(45);
    star->setName(systemNames[thx-1]);
    thestar->setPosition( Vector3d( myRandom(8000) * 1000000ll - 4000000000ll,
                                 myRandom(8000) * 1000000ll - 4000000000ll,
                                 0ll));
    star->addToParent( mw_galaxy->getID());
    obman->addObject( star);

    // Create a variable number of planets for each star system
    while ( nplanets < 5 && myRandom(10) < 6) {
        IGObject::Ptr  planet = game->getObjectManager()->createNewObject();
        formatter.str("");
        formatter << star->getName() << " " << nplanets;

        otypeman->setupObject(planet, obT_Planet);
        Planet* theplanet = dynamic_cast<Planet*>(planet->getObjectBehaviour());
        theplanet->setSize( 2);
        planet->setName( formatter.str().c_str());
        theplanet->setPosition( thestar->getPosition() + Vector3d( nplanets * 40000ll,
                                                             nplanets * -35000ll,
                                                             0ll));

        ResourceManager::Ptr resman = game->getResourceManager();
        std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
        ress[resman->getResourceDescription("Home Planet")->getResourceType()] =  std::pair<uint32_t, uint32_t>(1, 0);
        ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
        ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
        ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
        ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
        ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
        ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
        ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
        ress[resman->getResourceDescription("Factories")->getResourceType()] = std::pair<uint32_t, uint32_t>(game->getRandom()->getInRange(1, 15), 0);
        theplanet->setResources(ress);
        
        OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(planet->getParameterByType(obpT_Order_Queue));
        oqop->setQueueId( game->getOrderManager()->addOrderQueue(planet->getID(),0) );
        theplanet->setDefaultOrderTypes();

        planet->addToParent( star->getID());
        obman->addObject( planet);
        nplanets++;
    }

    Logger::getLogger()->debug( "Exiting MTSec::createStarSystem");
    return star;
}


// Create the Sol star system
IGObject::Ptr MTSec::createSolSystem( IGObject::Ptr mw_galaxy)
{
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    ResourceManager::Ptr resman = game->getResourceManager();
    IGObject::Ptr      sol = game->getObjectManager()->createNewObject();
    IGObject::Ptr      earth = game->getObjectManager()->createNewObject();
    IGObject::Ptr      venus = game->getObjectManager()->createNewObject();
    IGObject::Ptr      mars = game->getObjectManager()->createNewObject();
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    ObjectTypeManager* otypeman = game->getObjectTypeManager();
    
    uint32_t obT_Star_System = otypeman->getObjectTypeByName("Star System");
    uint32_t obT_Planet = otypeman->getObjectTypeByName("Planet");
    
    otypeman->setupObject(sol, obT_Star_System);
    EmptyObject* thesol = dynamic_cast<EmptyObject*>(sol->getObjectBehaviour());
    thesol->setSize(1400000ll);
    sol->setName("Sol/Terra System");
    thesol->setPosition(Vector3d(3000000000ll, 2000000000ll, 0ll));
    sol->addToParent(mw_galaxy->getID());
    obman->addObject(sol);

    otypeman->setupObject(earth, obT_Planet);
    Planet* theearth = dynamic_cast<Planet*>(earth->getObjectBehaviour());
    theearth->setSize(2);
    earth->setName("Earth/Terra");
    theearth->setPosition(thesol->getPosition() + Vector3d(14960ll, 0ll, 0ll));
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ress[resman->getResourceDescription("Factories")->getResourceType()] = std::pair<uint32_t, uint32_t>(game->getRandom()->getInRange(1, 15), 0);
    dynamic_cast<Planet*>(earth->getObjectBehaviour())->setResources(ress);
    
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(earth->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId( game->getOrderManager()->addOrderQueue(earth->getID(),0) );
    theearth->setDefaultOrderTypes();
    
    earth->addToParent(sol->getID());
    obman->addObject(earth);

    ress.clear();

    otypeman->setupObject(venus, obT_Planet);
    Planet* thevenus = dynamic_cast<Planet*>(venus->getObjectBehaviour());
    thevenus->setSize(2);
    venus->setName("Venus");
    thevenus->setPosition(thesol->getPosition() + Vector3d(0ll, 10800ll, 0ll));
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ress[resman->getResourceDescription("Factories")->getResourceType()] = std::pair<uint32_t, uint32_t>(game->getRandom()->getInRange(1, 15), 0);
    thevenus->setResources(ress);
    
    oqop = static_cast<OrderQueueObjectParam*>(venus->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId( game->getOrderManager()->addOrderQueue(venus->getID(),0) );
    thevenus->setDefaultOrderTypes();
    
    venus->addToParent(sol->getID());
    obman->addObject(venus);

    ress.clear();

    otypeman->setupObject(mars, obT_Planet);
    Planet* themars = dynamic_cast<Planet*>(mars->getObjectBehaviour());
    themars->setSize(1);
    mars->setName("Mars");
    themars->setPosition(thesol->getPosition() + Vector3d(-22790ll, 0ll, 0ll));
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ress[resman->getResourceDescription("Factories")->getResourceType()] = std::pair<uint32_t, uint32_t>(game->getRandom()->getInRange(1, 15), 0);
    themars->setResources(ress);
    
    oqop = static_cast<OrderQueueObjectParam*>(mars->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId( game->getOrderManager()->addOrderQueue(mars->getID(),0) );
    themars->setDefaultOrderTypes();
    
    mars->addToParent(sol->getID());
    obman->addObject(mars);

    return sol;
}


void MTSec::createResources(){
  Logger::getLogger()->debug( "Enter MTSec::createResources");

  ResourceManager::Ptr resman = Game::getGame()->getResourceManager();
  
  resman->addResourceDescription( "Ship part", "part", "Ships parts that can be used to create ships");
  resman->addResourceDescription( "Home Planet", "unit", "The home planet for a race.");
  
  ResourceDescription::Ptr res;

  res.reset( new ResourceDescription() );
  res->setNameSingular("Uranium");
  res->setNamePlural("Uranium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Uranium Ore.");
  res->setMass(1);
  res->setVolume(4);
  resman->addResourceDescription(res);
  
  res.reset( new ResourceDescription() );
  res->setNameSingular("Thorium");
  res->setNamePlural("Thorium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Thorium Ore.");
  res->setMass(1);
  res->setVolume(4);
  resman->addResourceDescription(res);
  
  res.reset( new ResourceDescription() );
  res->setNameSingular("Cerium");
  res->setNamePlural("Cerium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Cerium Ore.");
  res->setMass(1);
  res->setVolume(3);
  resman->addResourceDescription(res);
  
  res.reset( new ResourceDescription() );
  res->setNameSingular("Enriched Uranium");
  res->setNamePlural("Enriched Uranium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Enriched Uranium Ore.");
  res->setMass(1);
  res->setVolume(2);
  resman->addResourceDescription(res);
  
  res.reset( new ResourceDescription() );
  res->setNameSingular("Massivium");
  res->setNamePlural("Massivium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Massivium Ore.");
  res->setMass(1);
  res->setVolume(12);
  resman->addResourceDescription(res);
  
  res.reset( new ResourceDescription() );
  res->setNameSingular("Antiparticle");
  res->setNamePlural("Antiparticle");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Antiparticles in magnetic containment.");
  res->setMass(1);
  res->setVolume(1);
  resman->addResourceDescription(res);
  
  res.reset( new ResourceDescription() );
  res->setNameSingular("Antimatter");
  res->setNamePlural("Antimatter");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Antimatter in magnetic containment.");
  res->setMass(1);
  res->setVolume(1);
  resman->addResourceDescription(res);

  res.reset( new ResourceDescription() );
  res->setNameSingular("Factories");
  res->setNamePlural("Factories");
  res->setUnitSingular("unit");
  res->setUnitPlural("units");
  res->setDescription("Factories for production.");
  res->setMass(0);
  res->setVolume(0);
  resman->addResourceDescription(res);
  
  Logger::getLogger()->debug( "Exit MTSec::createResources");

}


void MTSec::createGame()
{
    Logger::getLogger()->debug( "Enter MTSec::createGame");
    Game*        game = Game::getGame();
    DesignStore::Ptr ds = game->getDesignStore();
    uint32_t counter;
    Category::Ptr cat( new Category() );
    cat->setName("Ships");
    cat->setDescription("The Ship design and component category");
    ds->addCategory(cat);
//  Don't think we need to asset that this category is 1 since we don't use the actual number 1 anywhere.
//    assert(cat->getCategoryId() == 1);

    cat.reset( new Category() );
    cat->setName("Weapons");
    cat->setDescription("The missile and torpedoe design and component category");
    ds->addCategory(cat);

    createTechTree();
    createResources();

    ObjectManager* obman = game->getObjectManager();
    ObjectTypeManager* otypeman = game->getObjectTypeManager();
    
    uint32_t obT_Universe = otypeman->getObjectTypeByName("Universe");
    uint32_t obT_Galaxy = otypeman->getObjectTypeByName("Galaxy");
    
    IGObject::Ptr universe = obman->createNewObject();
    otypeman->setupObject(universe, obT_Universe);
    Universe* theuniverse = dynamic_cast<Universe*>(universe->getObjectBehaviour());
    theuniverse->setSize(1000000000ll);
    universe->setName("The Universe");
    theuniverse->setPosition(Vector3d(0ll, 0ll, 0ll));
    obman->addObject(universe);

    //add contained objects
    IGObject::Ptr mw_galaxy = obman->createNewObject();
    otypeman->setupObject(mw_galaxy, obT_Galaxy);
    EmptyObject* themw = dynamic_cast<EmptyObject*>(mw_galaxy->getObjectBehaviour());
    themw->setSize(100000000ll);
    mw_galaxy->setName("Milky Way Galaxy");
    themw->setPosition(Vector3d(0ll, -6000ll, 0ll));
    mw_galaxy->addToParent(universe->getID());
    obman->addObject(mw_galaxy);

    // Some initial star systems...
    createSolSystem( mw_galaxy);
    createAlphaCentauriSystem( mw_galaxy);
    createSiriusSystem( mw_galaxy);
    for ( counter = 0; counter < 45; counter++) {
        createStarSystem( mw_galaxy);
    }

    Logger::getLogger()->debug( "Exit MTSec::createGame");

    return;
}


void MTSec::startGame()
{
  Settings* settings = Settings::getSettings();
  if(settings->get("turn_length_over_threshold") == ""){
    settings->set("turn_length_over_threshold", "600");
    settings->set("turn_player_threshold", "0");
    settings->set("turn_length_under_threshold", "600");
  }
}



bool MTSec::onAddPlayer(Player::Ptr player)
{
  return true;
}


Design::Ptr MTSec::createScoutDesign( Player::Ptr owner)
{
    Logger::getLogger()->debug( "Enter MTSec::createScoutDesign");
    Game *game = Game::getGame();
    DesignStore::Ptr ds = game->getDesignStore();
    Design::Ptr scout( new Design() );
    IdMap componentList;

    scout->setCategoryId(1);
    scout->setName( "Scout");
    scout->setDescription("Scout ship");
    scout->setOwner( owner->getID());
    componentList[ds->getComponentByName("Scout Hull")] = 1;
    componentList[ds->getComponentByName("Alpha Missile Tube")] = 1;
    scout->setComponents(componentList);
    game->getDesignStore()->addDesign(scout);
    Logger::getLogger()->debug( "Exit MTSec::createScoutDesign");

    return scout;
}


Design::Ptr MTSec::createBattleScoutDesign( Player::Ptr owner)
{
    Logger::getLogger()->debug( "Enter MTSec::createBattleScoutDesign");
    DesignStore::Ptr ds = Game::getGame()->getDesignStore();

    Game *game = Game::getGame();
    Design::Ptr scout( new Design() );
    IdMap componentList;

    scout->setCategoryId(1);
    scout->setName( "BattleScout");
    scout->setDescription("Battle Scout ship");
    scout->setOwner( owner->getID());
    componentList[ds->getComponentByName("Battle Scout Hull")] = 1;
    componentList[ds->getComponentByName("Alpha Missile Tube")] = 1;
    componentList[ds->getComponentByName("Cerium Explosives")] = 1;
    scout->setComponents(componentList);
    game->getDesignStore()->addDesign(scout);

    Logger::getLogger()->debug( "Exit MTSec::createBattleScoutDesign");
    return scout;
}

Design::Ptr MTSec::createAlphaMissileDesign( Player::Ptr owner)
{
    Logger::getLogger()->debug( "Enter MTSec::createAlphaMissileDesign");
    DesignStore::Ptr ds = Game::getGame()->getDesignStore();

    Game *game = Game::getGame();
    Design::Ptr scout( new Design() );
    IdMap componentList;

    scout->setCategoryId(2);
    scout->setName( "Alpha Missile");
    scout->setDescription("The Alpha Missile, Bitches");
    scout->setOwner( owner->getID());
    componentList[ds->getComponentByName("Alpha Missile Hull")] = 1;
    componentList[ds->getComponentByName("Cerium Explosives")] = 100;
    scout->setComponents(componentList);
    game->getDesignStore()->addDesign(scout);

    Logger::getLogger()->debug( "Exit MTSec::createAlphaMissileDesign");
    return scout;
}

IGObject::Ptr MTSec::createEmptyFleet( Player::Ptr     owner,
                                   IGObject::Ptr   star,
                                   std::string fleetName)
{
    Game *game = Game::getGame();
    ObjectTypeManager* otypeman = game->getObjectTypeManager();
    IGObject::Ptr fleet = game->getObjectManager()->createNewObject();
        
    otypeman->setupObject(fleet, otypeman->getObjectTypeByName("Fleet"));
    
    Vector3d  offset = Vector3d( ( int64_t) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 ( int64_t) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 /*(int64_t)((rand() % 10000) - 5000)*/ 0);
    
    Fleet* thefleet = dynamic_cast<Fleet*>(fleet->getObjectBehaviour());
    thefleet->setSize( 2);
    fleet->setName( fleetName.c_str());
    thefleet->setOwner(owner->getID());

    // Place the fleet in orbit around the given star
    thefleet->setPosition( dynamic_cast<EmptyObject*>(star->getObjectBehaviour())->getPosition() + offset);
    thefleet->setVelocity( Vector3d(0LL, 0ll, 0ll));
    
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(fleet->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId( game->getOrderManager()->addOrderQueue(fleet->getID(),owner->getID()) );
    thefleet->setDefaultOrderTypes();

    fleet->addToParent( star->getID());

    return fleet;
}


// A new player's initial fleet always consists of two battle scouts.
//
// The designs for the scouts are one-offs, as far as the player
// is concerned.  S/he is unable to produce any more ships like
// them, although they could create another design that functions
// similarly.
//
void MTSec::makeNewPlayerFleet( Player::Ptr player, IGObject::Ptr star)
{
    Logger::getLogger()->debug( "Enter MTSec::makeNewPlayerFleet");
    Game *game = Game::getGame();
    std::string fleetName = player->getName().substr( 0,11) + "'s Fleet #1";
    IGObject::Ptr   fleet = createEmptyFleet( player, star, fleetName);

    Design::Ptr scout = createScoutDesign( player);
    Design::Ptr tempMissile = createAlphaMissileDesign(player);

    PlayerView::Ptr playerview = player->getPlayerView();

    playerview->addUsableDesign(tempMissile->getDesignId());

    // Start this fleet off with two battle scout ships
    dynamic_cast<Fleet*>(fleet->getObjectBehaviour())->addShips( scout->getDesignId(), 2);
    scout->addUnderConstruction(2);
    scout->addComplete(2);
    game->getDesignStore()->designCountsUpdated( scout);

    game->getObjectManager()->addObject( fleet);

    Logger::getLogger()->debug( "Exit MTSec::makeNewPlayerFleet");
    return;
}


// Create a new player's home planet, orbiting around
// the given star.
IGObject::Ptr MTSec::makePlayerHomePlanet( Player::Ptr player, IGObject::Ptr star)
{
    Logger::getLogger()->debug( "Enter MTSec::makePlayerHomePlanet");
    Game *    game = Game::getGame();
    ObjectTypeManager* otypeman = game->getObjectTypeManager();
    IGObject::Ptr planet = game->getObjectManager()->createNewObject();
    
    std::string planetName = player->getName() + "'s Planet";

    uint32_t obT_Planet = otypeman->getObjectTypeByName("Planet");
    
    otypeman->setupObject(planet, obT_Planet);
    Planet* theplanet = dynamic_cast<Planet*>(planet->getObjectBehaviour());
    Vector3d  offset = Vector3d( ( int64_t) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 ( int64_t) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 /*(int64_t)((rand() % 10000) - 5000)*/ 0);
    theplanet->setSize( 2);
    planet->setName( planetName.c_str());
    theplanet->setOwner(player->getID());
    theplanet->setPosition( dynamic_cast<EmptyObject*>(star->getObjectBehaviour())->getPosition() + offset);

    ResourceManager::Ptr resman = game->getResourceManager();
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    ress[resman->getResourceDescription("Home Planet")->getResourceType()] =  std::pair<uint32_t, uint32_t>(1, 0);
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ress[resman->getResourceDescription("Factories")->getResourceType()] = std::pair<uint32_t, uint32_t>(game->getRandom()->getInRange(20, 50), 0);
    theplanet->setResources(ress);
    
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(planet->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId( game->getOrderManager()->addOrderQueue(planet->getID(), player->getID()) );
    theplanet->setDefaultOrderTypes();

    planet->addToParent( star->getID());
    game->getObjectManager()->addObject(planet);

    Logger::getLogger()->debug( "Exit MTSec::makePlayerHomePlanet");
    return planet;
}


// Create a 'home star system' for a new player
//
// These 'home systems' always consist of exactly one planet.
//
IGObject::Ptr MTSec::makeNewPlayerStarSystem( Player::Ptr player)
{
    Logger::getLogger()->debug( "Enter MTSec::makeNewPlayerStarSystem");
    Game *    game = Game::getGame();
    ObjectTypeManager* otypeman = game->getObjectTypeManager();
    IGObject::Ptr star = game->getObjectManager()->createNewObject();
    
    std::string starName = player->getName() + " Star System";

    otypeman->setupObject(star, otypeman->getObjectTypeByName("Star System"));
    Vector3d  location = Vector3d( ( int64_t) ( ( game->getRandom()->getInRange(0, 1000) - 500) * 10000000),
                                   ( int64_t) ( ( game->getRandom()->getInRange(0, 1000) - 500) * 10000000),
                                   /*(int64_t)(((rand()%1000)-500)*10000000)*/ 0);
    EmptyObject* thestar = dynamic_cast<EmptyObject*>(star->getObjectBehaviour());
    thestar->setSize( 2000000ll);
    star->setName( starName.c_str());
    thestar->setPosition( location);

    star->addToParent(1);
    game->getObjectManager()->addObject( star);

    makePlayerHomePlanet( player, star);

    Logger::getLogger()->debug( "Exit MTSec::makeNewPlayerStarSystem");
    return star;
}


// This routine sets a new player's initial tech level.
// New players can make any hull, see all stars.  This
// doesn't leave anything for them to research.
void MTSec::setNewPlayerTech( Player::Ptr player)
{
    Logger::getLogger()->debug( "Enter MTSec::setNewPlayerTech");
    Game *game = Game::getGame();

    PlayerView::Ptr playerview = player->getPlayerView();
    std::set<uint32_t> objids = game->getObjectManager()->getAllIds();
    for(std::set<uint32_t>::iterator itcurr = objids.begin(); itcurr != objids.end();
        ++itcurr){
      playerview->addVisibleObject( *itcurr, true );
    }

   for(uint32_t itcurr = 1; itcurr <= game->getDesignStore()->getMaxComponentId(); ++itcurr){
      playerview->addUsableComponent(itcurr);
   }

    Logger::getLogger()->debug( "Exit MTSec::setNewPlayerTech");
    return;
}


// New players start with their own star systems, and an
// initial fleet consisting of two scout ships.
//
void MTSec::onPlayerAdded(Player::Ptr player)
{
    Logger::getLogger()->debug( "Enter MTSec::onPlayerAdded");

    IGObject::Ptr star = makeNewPlayerStarSystem( player);

    setNewPlayerTech( player);
    makeNewPlayerFleet( player, star);
    Game::getGame()->getPlayerManager()->updatePlayer( player->getID());
    
    PlayerView::Ptr playerview = player->getPlayerView();
    std::set<uint32_t> objids = Game::getGame()->getObjectManager()->getAllIds();
    for(std::set<uint32_t>::iterator itcurr = objids.begin(); itcurr != objids.end();
        ++itcurr){
      playerview->addVisibleObject( *itcurr, true );
    }

    Logger::getLogger()->debug( "Exit MTSec::onPlayerAdded");
    return;
}

}//end namespace

