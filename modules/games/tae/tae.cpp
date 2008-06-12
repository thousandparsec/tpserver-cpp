/*  TaE ruleset
 *
 *  Copyright (C) 2008  Dustin White and the Thousand Parsec Project
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

//System includes
#include <sstream>

//tpserver includes
#include <tpserver/game.h>
#include <tpserver/logging.h>
#include <tpserver/object.h>
#include <tpserver/objectview.h>
#include <tpserver/objectmanager.h>
#include <tpserver/objecttypemanager.h>
#include <tpserver/ordermanager.h>
#include <tpserver/orderqueue.h>
#include <tpserver/orderqueueobjectparam.h>
#include <tpserver/player.h>
#include <tpserver/playerview.h>
#include <tpserver/playermanager.h>
#include <tpserver/design.h>
#include <tpserver/designstore.h>
#include <tpserver/designview.h>
#include <tpserver/resourcemanager.h>
#include <tpserver/resourcedescription.h>
#include <tpserver/category.h>
#include <tpserver/property.h>
#include <tpserver/component.h>

//tae includes
#include "universe.h"
#include "emptyobject.h"
#include "spaceobject.h"
#include "starsystem.h"
#include "planet.h"
#include "fleet.h"
#include "colonize.h"
#include "move.h"
#include "taeturn.h"

//header includes
#include "tae.h"

using namespace tae;
using std::map;
using std::string;

taeRuleset::taeRuleset() {
}

taeRuleset::~taeRuleset() {
}

std::string taeRuleset::getName() {
    return "TaE";
}

std::string taeRuleset::getVersion() {
    return "0.01";
}

void taeRuleset::initGame() {
    Game* game = Game::getGame();

    //Setup turns
    TaeTurn* turn = new TaeTurn();
    game->setTurnProcess(turn);

    //Seed rand
    std::srand(std::time(NULL));

    //Set default ships left
    shipsLeft[0] = 30;  //Merchants
    shipsLeft[1] = 57;  //Scientists
    shipsLeft[2] = 30;  //Settlers
    shipsLeft[3] = 36;  //Miners

    //Add universe object type
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    obtm->addNewObjectType(new UniverseType());

    //Add Galaxy object type
    EmptyObjectType * eo = new EmptyObjectType();
    eo->setTypeName("Galaxy");
    eo->setTypeDescription("The Galaxy Object type");
    obtm->addNewObjectType(eo);

    //Add Solar system object type
    obtm->addNewObjectType(new StarSystemType());

    //Add Planet object type
    uint32_t pt = obtm->addNewObjectType(new PlanetType());
    turn->setPlanetType(pt);

    //Add Fleet object type
    uint32_t ft = obtm->addNewObjectType(new FleetType());
    turn->setFleetType(ft);
    
    //Set Order types
    OrderManager* orm = game->getOrderManager();
    orm->addOrderType(new Colonize());
    orm->addOrderType(new Move());

    Logger::getLogger()->info("TaE initialised");
}

void taeRuleset::createGame() {
    Game* game = Game::getGame();
    ObjectManager* obm = game->getObjectManager();
    ObjectTypeManager* obtm = game->getObjectTypeManager();

    //Create ship design category
    Category* cat = new Category();
    cat->setName("Ships");
    cat->setDescription("Ship components");
    game->getDesignStore()->addCategory(cat);
    
    //Create properties
    createProperties();

    //Create components
    createComponents();

    //Create resources
    setupResources();

    uint32_t obT_Universe = obtm->getObjectTypeByName("Universe");
    uint32_t obT_Galaxy = obtm->getObjectTypeByName("Galaxy");
    uint32_t obT_Star_System = obtm->getObjectTypeByName("Star System");
    uint32_t obT_Planet = obtm->getObjectTypeByName("Planet");

    //Create the universe
    IGObject* universe = obm->createNewObject();
    obtm->setupObject(universe, obT_Universe);
    Universe* theUniverse = (Universe*)(universe->getObjectBehaviour());
    theUniverse->setSize(1000000000000ll);
    universe->setName("The Universe");
    theUniverse->setPosition(Vector3d(0ll,0ll,0ll));
    obm->addObject(universe);

    //Create the galaxy
    IGObject* gal = obm->createNewObject();
    obtm->setupObject(gal, obT_Galaxy);
    EmptyObject* galob = (EmptyObject*)(gal->getObjectBehaviour());
    galob->setSize(100000000000ll);
    gal->setName("The Fertile Galaxy");
    galob->setPosition(Vector3d(0ll, -6000ll, 0ll));
    gal->addToParent(universe->getID());
    obm->addObject(gal);

    //Create the "Board" of star systems
    for(int i = 0; i < 11; i++) {
        for(int j = 0; j < 16; j++) {
            //Create a star system
            IGObject* sys1 = obm->createNewObject();
            obtm->setupObject(sys1, obT_Star_System);
            StarSystem* sys1ob = (StarSystem*)(sys1->getObjectBehaviour());
            sys1ob->setSize(60000ll);
            char* name = new char[8];
            sprintf(name, "%d, %d", j, i);
            sys1->setName(name);
            sys1ob->setPosition(Vector3d(1ll + 80000ll*j, 1ll+80000ll*(i+1), 0ll));
            sys1ob->setInhabitable(true);
            sys1ob->setDestroyed(true);
            sys1->addToParent(gal->getID());
            obm->addObject(sys1);

            //Create a planet
            IGObject *p = obm->createNewObject();
            obtm->setupObject(p, obT_Planet);
            Planet * pob = (Planet*)(p->getObjectBehaviour());
            pob->setSize(2);
            char* planetName = new char[20];
            sprintf(planetName, "Planet %d, %d", j, i);
            p->setName(planetName);
            pob->setPosition(sys1ob->getPosition());
            OrderQueue *planetoq = new OrderQueue();
            planetoq->setObjectId(p->getID());
            planetoq->addOwner(0);
            game->getOrderManager()->addOrderQueue(planetoq);
            OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(p->getParameterByType(obpT_Order_Queue));
            oqop->setQueueId(planetoq->getQueueId());
            pob->setDefaultOrderTypes();
            p->addToParent(sys1->getID());
            obm->addObject(p);
        }
    }


    Logger::getLogger()->info("TaE created");
}

void taeRuleset::setupResources() {
    ResourceManager* rman = Game::getGame()->getResourceManager();
    //Setup Inhabitable resource
    if(rman->getResourceDescription(1) == NULL){
        Logger::getLogger()->info("Setting up Resource: Uninhabitable");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Uninhabitable planet");
        res->setNamePlural("Uninhabitable planets");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("Planets that can only be colonized by mining robots");
        res->setMass(0);
        res->setVolume(0);
        rman->addResourceDescription(res);
    }
    //Setup Destroyed resource
    if(rman->getResourceDescription(2) == NULL){
        Logger::getLogger()->info("Setting up Resource: Destroyed");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Destroyed Star System");
        res->setNamePlural("Destroyed Star Systems");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("Star systems that have been destroyed");
        res->setMass(0);
        res->setVolume(0);
        rman->addResourceDescription(res);
    }
    //Setup Artifact resource
    if(rman->getResourceDescription(3) == NULL){
        Logger::getLogger()->info("Setting up Resource: Artifact");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Alien Artifact");
        res->setNamePlural("Alien Artifacts");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("Ancient alien artifacts of great value");
        res->setMass(0);
        res->setVolume(0);
        rman->addResourceDescription(res);
    }
    //Setup Money resource
    if(rman->getResourceDescription(4) == NULL){
        Logger::getLogger()->info("Setting up Resource: Money");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Monetary Asset");
        res->setNamePlural("Monetary Assets");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("Valuable investments in successful merchant colonies");
        res->setMass(0);
        res->setVolume(0);
        rman->addResourceDescription(res);
    }
    //Setup Technology resource
    if(rman->getResourceDescription(5) == NULL){
        Logger::getLogger()->info("Setting up Resource: Technology");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Technological Advancement");
        res->setNamePlural("Technological Advancements");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("Advances in technology produced by brilliant scientists");
        res->setMass(0);
        res->setVolume(0);
        rman->addResourceDescription(res);
    }
    //Setup People resource
    if(rman->getResourceDescription(6) == NULL){
        Logger::getLogger()->info("Setting up Resource: People");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Team of People");
        res->setNamePlural("Teams of People");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("Eager manpower produced by loyal colonists");
        res->setMass(0);
        res->setVolume(0);
        rman->addResourceDescription(res);
    }
    //Setup Raw Material resource
    if(rman->getResourceDescription(7) == NULL){
        Logger::getLogger()->info("Setting up Resource: Raw Materials");
        ResourceDescription* res = new ResourceDescription();
        res->setNameSingular("Raw Material");
        res->setNamePlural("Raw Materials");
        res->setUnitSingular("unit");
        res->setUnitPlural("units");
        res->setDescription("Essential raw materials harvested by mining robots");
        res->setMass(0);
        res->setVolume(0);
        rman->addResourceDescription(res);
    }
}

// Create properties for use with components.  This function is based off the
// function of the same name implemented in RFTS.
void taeRuleset::createProperties() {
    Property* prop = new Property();
    DesignStore* ds = Game::getGame()->getDesignStore();

    // Passengers
    prop = new Property();
    prop->addCategoryId(ds->getCategoryByName("Ships"));
    prop->setRank(0);
    prop->setName("Passengers");
    prop->setDisplayName("Passengers");
    prop->setDescription("The passengers aboard the ship");
    // Value of property -> String to display:
    // 1 -> Merchants
    // 2 -> Scientists
    // 3 -> Settlers
    // 4 -> Mining Robots
    // 5 -> Merchant Leader
    // 6 -> Lead Scientist
    // 7 -> Government Official
    // 8 -> Mining Foreman
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits)))"
        "(cons n (cond "
            "((= n 1) \"Merchants\") "
            "((= n 2) \"Scientists\") "
            "((= n 3) \"Settlers\") "
            "((= n 4) \"Mining Robots\") "
            "((= n 5) \"Merchant Leader\") "
            "((= n 6) \"Lead Scientist\") "
            "((= n 7) \"Government Official\") "
            "((= n 8) \"Mining Foreman\") "
            "((< n 1) (cons n \"ERROR: value too low!\")) "
            "((> n 8) (cons n \"ERROR: value too high!\"))))))");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);

    prop = new Property();
    prop->addCategoryId(ds->getCategoryByName("Ships"));
    prop->setRank(0);
    prop->setName("Bombs");
    prop->setDisplayName("Supernova Bombs");
    prop->setDescription("The number of supernova bombs aboard the ship");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" bombs\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);

}

void taeRuleset::createComponents() {
    DesignStore *ds = Game::getGame()->getDesignStore();

    Component* comp = new Component();
    map<unsigned int, string> propList;

    //Merchants
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("MerchantCargo");
    comp->setDescription("A cargo hold outfitted to carry businessmen");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 1)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Scientists
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("ScientistCargo");
    comp->setDescription("A cargo hold outfitted to carry scientists");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 2)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Settlers
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("SettlerCargo");
    comp->setDescription("A cargo hold outfitted to carry settlers");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 3)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Mining Robots
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("MiningCargo");
    comp->setDescription("A cargo hold outfitted to carry mining robots");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 4)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Merchant Leader
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("MerchantLeaderCargo");
    comp->setDescription("A cargo hold outfitted to carry powerful business leaders");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 5)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Lead Scientist
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("ScientistLeaderCargo");
    comp->setDescription("A cargo hold outfitted to carry a lead scientist");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 6)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Government Official
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("SettlerLeaderCargo");
    comp->setDescription("A cargo hold outfitted to a government leader");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 7)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Mining Foreman
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("MiningLeaderCargo");
    comp->setDescription("A cargo hold outfitted to a highly specialized mining robot leader");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Passengers")] = "(lambda (design) 8)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

    //Weapons Arsenal
    comp = new Component();
    comp->addCategoryId(ds->getCategoryByName("Ships"));
    comp->setName("Weapon");
    comp->setDescription("The weapons arsenal for the ship");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propList.clear();
    propList[ds->getPropertyByName("Bombs")] = "(lambda (design) 1)";
    comp->setPropertyList(propList);
    ds->addComponent(comp);

}

Design* taeRuleset::createPassengerShip(Player* owner, int type) {
    Design* ship = new Design();
    map<unsigned int, unsigned int> componentList;

    DesignStore * ds = Game::getGame()->getDesignStore();
    
    ship->setCategoryId(ds->getCategoryByName("Ships"));
    ship->setDescription("A passenger transport ship");
    ship->setOwner(owner->getID());
    //TODO: I hate if/else statements like this... I may change this later
    if(type == 1) {
        ship->setName("MerchantShip");
        componentList[ds->getComponentByName("MerchantCargo")] = 1;
    } else if (type == 2) {
        ship->setName("ScientistShip");
        componentList[ds->getComponentByName("ScientistCargo")] = 1;
    } else if (type == 3) {
        ship->setName("SettlerShip");
        componentList[ds->getComponentByName("SettlerCargo")] = 1;
    } else {
        ship->setName("MiningShip");
        componentList[ds->getComponentByName("MiningCargo")] = 1;
    }
    ship->setComponents(componentList);
    ds->addDesign(ship);

    return ship;
}

Design* taeRuleset::createRandomPassengerShip(Player* owner) {
    int type;

    //Check to see if there are any ships left
    if((shipsLeft[0] == 0) && (shipsLeft[1] == 0) && (shipsLeft[2] == 0) && (shipsLeft[3]== 0)) {
        //TODO: initiate game over sequence
    }

    //Select a ship type
    do {
        type = std::rand() % 4;
    } while(shipsLeft[type] <= 0);

    shipsLeft[type]--;

    return createPassengerShip(owner, type);
}

Design* taeRuleset::createVIPTransport(Player* owner, int type) {
    Design* ship = new Design();
    map<unsigned int, unsigned int> componentList;

    DesignStore * ds = Game::getGame()->getDesignStore();
    
    ship->setCategoryId(ds->getCategoryByName("Ships"));
    ship->setDescription("A passenger transport ship for VIPs");
    ship->setOwner(owner->getID());
    //TODO: I hate if/else statements like this... I may change this later
    if(type == 1) {
        ship->setName("MerchantLeaderShip");
        componentList[ds->getComponentByName("MerchantLeaderCargo")] = 1;
    } else if (type == 2) {
        ship->setName("ScientistLeaderShip");
        componentList[ds->getComponentByName("ScientistLeaderCargo")] = 1;
    } else if (type == 3) {
        ship->setName("SettlerLeaderShip");
        componentList[ds->getComponentByName("SettlerLeaderCargo")] = 1;
    } else {
        ship->setName("MiningLeaderShip");
        componentList[ds->getComponentByName("MiningLeaderCargo")] = 1;
    }
    ship->setComponents(componentList);
    ds->addDesign(ship);

    return ship;
}

Design* taeRuleset::createBomber(Player* owner) {
    Design* ship = new Design();
    map<unsigned int, unsigned int> componentList;

    DesignStore * ds = Game::getGame()->getDesignStore();
    
    ship->setCategoryId(ds->getCategoryByName("Ships"));
    ship->setName("Bomber");
    ship->setDescription("A bomber capable of destroying star systems");
    ship->setOwner(owner->getID());
    componentList[ds->getComponentByName("Weapon")] = 1;
    ship->setComponents(componentList);
    ds->addDesign(ship);

    return ship;
}

//Creates an empty fleet owned by "owner" at the location of "parent"
//Adapted from the createEmptyFleet function of mtsec
IGObject* taeRuleset::createEmptyFleet(Player* owner, IGObject* parent, string name) {
    Game *game = Game::getGame();
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    IGObject *fleet = game->getObjectManager()->createNewObject();
    obtm->setupObject(fleet, obtm->getObjectTypeByName("Fleet"));
    
    Fleet* theFleet = (Fleet*) (fleet->getObjectBehaviour());
    theFleet->setSize(2);
    fleet->setName(name.c_str());
    theFleet->setOwner(owner->getID());

    theFleet->setPosition(((SpaceObject*)(parent->getObjectBehaviour()))->getPosition());
    theFleet->setVelocity(Vector3d(0ll,0ll,0ll));

    OrderQueue *fleetoq = new OrderQueue();
    fleetoq->setQueueId(fleet->getID());
    fleetoq->addOwner(owner->getID());
    game->getOrderManager()->addOrderQueue(fleetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(fleet->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(fleetoq->getQueueId());
    theFleet->setDefaultOrderTypes();

    fleet->addToParent(parent->getID());
    return fleet;
}

//Creates an empty fleet owned by "owner" at the location specified by "loc"
//Adapted from the createEmptyFleet function of mtsec
IGObject* taeRuleset::createEmptyFleet(Player* owner, Vector3d loc, string name) {
    Game *game = Game::getGame();
    ObjectTypeManager* obtm = game->getObjectTypeManager();
    IGObject *fleet = game->getObjectManager()->createNewObject();
    obtm->setupObject(fleet, obtm->getObjectTypeByName("Fleet"));
    
    Fleet* theFleet = (Fleet*) (fleet->getObjectBehaviour());
    theFleet->setSize(2);
    fleet->setName(name.c_str());
    theFleet->setOwner(owner->getID());

    theFleet->setPosition(loc);
    theFleet->setVelocity(Vector3d(0ll,0ll,0ll));

    OrderQueue *fleetoq = new OrderQueue();
    fleetoq->setQueueId(fleet->getID());
    fleetoq->addOwner(owner->getID());
    game->getOrderManager()->addOrderQueue(fleetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(fleet->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(fleetoq->getQueueId());
    theFleet->setDefaultOrderTypes();

    fleet->addToParent(1);
    return fleet;
}

void taeRuleset::startGame() {
    setupResources();
    Logger::getLogger()->info("TaE started");
}

bool taeRuleset::onAddPlayer(Player* player) {
    Logger::getLogger()->debug("TaE onAddPlayer");
    return true;
}

void taeRuleset::onPlayerAdded(Player* player) {
    Logger::getLogger()->debug("TaE onPlayerAdded");

    Game *game = Game::getGame();
    PlayerView* playerview = player->getPlayerView();
    ObjectTypeManager* obtm = game->getObjectTypeManager();

    std::set<uint32_t> allotherdesigns = Game::getGame()->getDesignStore()->getDesignIds();
    for(std::set<uint32_t>::const_iterator desid = allotherdesigns.begin(); desid != allotherdesigns.end(); ++desid){
        DesignView* dv = new DesignView();
        dv->setDesignId(*desid);
        dv->setIsCompletelyVisible(true);
        playerview->addVisibleDesign(dv);
    }

   for(uint32_t i = 1; i <= game->getDesignStore()->getMaxComponentId(); ++i){
      playerview->addUsableComponent(i);
   }

   std::set<uint32_t> mydesignids;

    //Add system and planet to hold player's fleets
    IGObject* sys1 = game->getObjectManager()->createNewObject();
    obtm->setupObject(sys1, obtm->getObjectTypeByName("Star System"));
    StarSystem* sys1ob = (StarSystem*)(sys1->getObjectBehaviour());
    sys1ob->setSize(60000ll);
    sys1->setName(player->getName() + "'s System");
    sys1ob->setPosition(Vector3d(1ll+80000*player->getID(), 1ll, 0ll));
    sys1ob->setInhabitable(true);
    sys1ob->setDestroyed(true);
    sys1->addToParent(1);
    game->getObjectManager()->addObject(sys1);

    IGObject *p = game->getObjectManager()->createNewObject();
    obtm->setupObject(p, obtm->getObjectTypeByName("Planet"));
    Planet * pob = (Planet*)(p->getObjectBehaviour());
    pob->setSize(2);
    p->setName(player->getName() + "'s Home Planet");
    pob->setPosition(sys1ob->getPosition());
    OrderQueue *planetoq = new OrderQueue();
    planetoq->setObjectId(p->getID());
    planetoq->addOwner(player->getID());
    game->getOrderManager()->addOrderQueue(planetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(p->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(planetoq->getQueueId());
    pob->setDefaultOrderTypes();
    p->addToParent(sys1->getID());
    game->getObjectManager()->addObject(p);

    //Setup starting fleets
    //Colonist fleets
    IGObject* fleet = createEmptyFleet(player, p, "Colonist Fleet");
    Design* ship = createRandomPassengerShip(player);
    Fleet* fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Colonize");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Colonist Fleet");
    ship = createRandomPassengerShip(player);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Colonize");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Colonist Fleet");
    ship = createRandomPassengerShip(player);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Colonize");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Colonist Fleet");
    ship = createRandomPassengerShip(player);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Colonize");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Colonist Fleet");
    ship = createRandomPassengerShip(player);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Colonize");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Colonist Fleet");
    ship = createRandomPassengerShip(player);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Colonize");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    //Leader fleets
    fleet = createEmptyFleet(player, p, "Merchant Leader");
    ship = createVIPTransport(player, 1);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Move");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Scientist Leader");
    ship = createVIPTransport(player, 2);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Move");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Settler Leader");
    ship = createVIPTransport(player, 3);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Move");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Mining Leader");
    ship = createVIPTransport(player, 4);
    fleetObj =(Fleet*)(fleet->getObjectBehaviour());
    fleetObj->addShips(ship->getDesignId(), 1);
    fleetObj->addAllowedOrder("Move");
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    //Bomber fleets
    fleet = createEmptyFleet(player, p, "Bomber");
    ship = createBomber(player);
    ((Fleet*)(fleet->getObjectBehaviour()))->addShips(ship->getDesignId(), 1);
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);

    fleet = createEmptyFleet(player, p, "Bomber");
    ship = createBomber(player);
    ((Fleet*)(fleet->getObjectBehaviour()))->addShips(ship->getDesignId(), 1);
    game->getDesignStore()->designCountsUpdated(ship);
    mydesignids.insert(ship->getDesignId());
    game->getObjectManager()->addObject(fleet);


    //Make designs visible
    std::set<uint32_t> objids = game->getObjectManager()->getAllIds();
    for(std::set<uint32_t>::iterator itcurr = objids.begin(); itcurr != objids.end(); ++itcurr){
        ObjectView* obv = new ObjectView();
        obv->setObjectId(*itcurr);
        obv->setCompletelyVisible(true);
        playerview->addVisibleObject(obv);
    }

   std::set<uint32_t> playerids = game->getPlayerManager()->getAllIds();
    for(std::set<uint32_t>::iterator playerit = playerids.begin(); playerit != playerids.end(); ++playerit){
      if(*playerit == player->getID())
        continue;

      Player* oplayer = game->getPlayerManager()->getPlayer(*playerit);
      for(std::set<uint32_t>::const_iterator desid = mydesignids.begin(); desid != mydesignids.end(); ++desid){
        DesignView* dv = new DesignView();
        dv->setDesignId(*desid);
        dv->setIsCompletelyVisible(true);
        oplayer->getPlayerView()->addVisibleDesign(dv);
      }
      game->getPlayerManager()->updatePlayer(oplayer->getID());
    }

}

extern "C" {
#define tp_init libtae_LTX_tp_init
    bool tp_init() {
        return Game::getGame()->setRuleset(new tae::taeRuleset());
    }
}
