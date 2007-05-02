/*  MTSec ruleset
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include "tpserver/objectmanager.h"
#include "tpserver/ownedobject.h"
#include "universe.h"
#include "emptyobject.h"
#include "planet.h"
#include "fleet.h"
#include "tpserver/objectdatamanager.h"
#include "tpserver/player.h"
#include "avacombat.h"
#include "tpserver/designstore.h"
#include "tpserver/ordermanager.h"
#include "nop.h"
#include "move.h"
#include "build.h"
#include "colonise.h"
#include "splitfleet.h"
#include "mergefleet.h"
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
#include "mtsecturn.h"

#include "mtsec.h"

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
    return Game::getGame()->setRuleset(new MTSec());
  }
}


MTSec::MTSec() {

}


MTSec::~MTSec() {

}


std::string MTSec::getName(){
  return "MTSec";
}

std::string MTSec::getVersion(){
  return "0.0";
}

void MTSec::initGame() {
    Game* game = Game::getGame();

    MTSecTurn* turn = new MTSecTurn();
    
    game->setTurnProcess(turn);

    ObjectDataManager* obdm = game->getObjectDataManager();
    obdm->addNewObjectType( new Universe());
    EmptyObject * eo = new EmptyObject();
    eo->setTypeName("Galaxy");
    eo->setTypeDescription("The Galaxy Object type");
    obdm->addNewObjectType(eo);
    eo = new EmptyObject();
    eo->setTypeName("Star System");
    eo->setTypeDescription("The Star System Object type");
    obdm->addNewObjectType(eo);
    uint32_t pt = obdm->addNewObjectType( new Planet());
    uint32_t ft = obdm->addNewObjectType( new Fleet());
    
    turn->setPlanetType(pt);
    turn->setFleetType(ft);

    OrderManager* ordm = game->getOrderManager();
    ordm->addOrderType(new Nop());
    ordm->addOrderType(new Move());
    ordm->addOrderType(new Build());
    ordm->addOrderType(new Colonise());
    ordm->addOrderType(new SplitFleet());
    ordm->addOrderType(new MergeFleet());

    compMax = 13;
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


void MTSec::createSpeedProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("Speed");
    prop->setDisplayName("Speed");
    prop->setDescription("The number of units the ship can move each turn");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string (/ n 1000000)) \" mega-units\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createAmmoCostProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("AmmoCost");
    prop->setDisplayName("Explosive Unit Cost");
    prop->setDescription("The relative expensiveness of the explosive");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" credits\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createAmmoExplosivenessProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("AmmoExplosiveness");
    prop->setDisplayName("Unit Explosive Punch");
    prop->setDescription("The explosiveness of a unit of the explosive used in the weapons of the ship");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" hit points\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createAmmoSizeProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("AmmoSize");
    prop->setDisplayName("Explosive Density");
    prop->setDescription("The relative size of a unit of the explosive used in the weapons of the ship");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (number->string n))))");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createFirepowerProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(2);
    prop->setName("Firepower");
    prop->setDisplayName("Firepower");
    prop->setDescription("How much damage the weapons of the ship can do in a single round");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" hit points\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createMissileCostProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(1);
    prop->setName("MissileCost");
    prop->setDisplayName("MissileCost");
    prop->setDescription("The relative cost of a missile");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (number->string n))))");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createMissileFirepowerProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(1);
    prop->setName("MissileFirepower");
    prop->setDisplayName("Missile Firepower");
    prop->setDescription("How much damage a missile can do");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" hit points\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createMissileSizeProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(1);
    prop->setName("MissileSize");
    prop->setDisplayName("MissileSize");
    prop->setDescription("The relative size of a missile");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (number->string n))))");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createHitPointsProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("StartingHitPoints");
    prop->setDisplayName("Initial Hit Points");
    prop->setDescription("How much damage the ship can take before being destroyed");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" hit points\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createBuildTimeProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("BuildTime");
    prop->setDisplayName("Build Time");
    prop->setDescription("The number of turns to build this");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" turns\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createArmorProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("Armor");
    prop->setDisplayName("Armor");
    prop->setDescription("The amount of armor on the ship");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createHPProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("HitPoints");
    prop->setDisplayName("Hit Points");
    prop->setDescription("The number of HP this ship has");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string n) \" HP\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createColoniseProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("Colonise");
    prop->setDisplayName("Can Colonise Planets");
    prop->setDescription("Can the ship colonise planets");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (if (= n 1) \"Yes\" \"No\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createNumAmmoProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("num-ammo");
    prop->setDisplayName("Number of missile and torpedo warhead types");
    prop->setDescription("The number of missile and torpedo warhead types in the design");
    prop->setTpclDisplayFunction(
        "(lambda (design bits)"
        "(let ((n (apply + bits)))"
        "(cons n (string-append (number->string n) \" explosive types\"))))");
        prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createNumBayTypesProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("num-baytypes");
    prop->setDisplayName("Number of missile or torpedo types");
    prop->setDescription("The number of missile or torpedo types in the design");
    prop->setTpclDisplayFunction(
        "(lambda (design bits)"
        "(let ((n (apply + bits)))"
        "(cons n (string-append (number->string n) \" types\"))))");
        prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createNumHullsProp()
{
    Property* prop = new Property();
    DesignStore *ds = Game::getGame()->getDesignStore();

    prop->setCategoryId(1);
    prop->setRank(0);
    prop->setName("num-hulls");
    prop->setDisplayName("Number of hulls");
    prop->setDescription("The number of hulls in the design");
    prop->setTpclDisplayFunction(
        "(lambda (design bits)"
        "(let ((n (apply + bits)))"
        "(cons n (string-append (number->string n) \" hull\"))))");
        prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    ds->addProperty(prop);
    propertyIndex[prop->getName()] = prop->getPropertyId();

    return;
}


void MTSec::createScoutHullComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "ScoutHull");
    comp->setDescription( "The scout hull, fitted out with everything a scout needs");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) (* 100 1000000))";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 2)";
    propertylist[propertyIndex["_num-components"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createBattleScoutHullComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "BattleScoutHull");
    comp->setDescription( "The battle scout hull");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-hulls design) 1) "
                "(if (< (designType.num-baytypes design) 2) "
                    "(cons #t \"\") "
                    "(cons #f \"A ship can only have one type of missile bay\")) "
                "(cons #f \"A ship can only use one hull!\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) (* 75 1000000))";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 5)";
    // To determine the total firepower of the ship, we multiply the
    // firepower of an individual missile bay by how many missile bays
    // the ship has.
    // To determine how many missile bays the ship can have, we divide
    // the ship size by the missile size.
    propertylist[propertyIndex["Firepower"]] = "(lambda (design) "
        "(* (floor (/ 88 (designType.MissileSize design))) (designType.MissileFirepower design)))";
    propertylist[propertyIndex["HitPoints"]] = "(lambda (design) 100)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-hulls"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createCerium3AmmoComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "Cerium3Explosives");
    comp->setDescription( "A huge but extremely explosive sub-nuclear particle");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["AmmoSize"]] = "(lambda (design) 3)";
    propertylist[propertyIndex["AmmoExplosiveness"]] = "(lambda (design) 8)";
    // Ammo cost is (cost/size) * size.
    // This is for one 'unit' of explosive - if a missile can fit two units,
    // the cost for the missile bay is twice the AmmoCost property
    propertylist[propertyIndex["AmmoCost"]] = "(lambda (design) 14)";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createCerium6AmmoComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "Cerium6Explosives");
    comp->setDescription( "A huge but extremely explosive sub-nuclear particle");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["AmmoSize"]] = "(lambda (design) 6)";
    propertylist[propertyIndex["AmmoExplosiveness"]] = "(lambda (design) 11)";
    // Ammo cost is (cost/size) * size.
    // This is for one 'unit' of explosive - if a missile can fit two units,
    // the cost for the missile bay is twice the AmmoCost property
    propertylist[propertyIndex["AmmoCost"]] = "(lambda (design) 36)";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createCerium12AmmoComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "Cerium12Explosives");
    comp->setDescription( "A huge but extremely explosive sub-nuclear particle");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["AmmoSize"]] = "(lambda (design) 12)";
    propertylist[propertyIndex["AmmoExplosiveness"]] = "(lambda (design) 55)";
    // Ammo cost is (cost/size) * size.
    // This is for one 'unit' of explosive - if a missile can fit two units,
    // the cost for the missile bay is twice the AmmoCost property
    propertylist[propertyIndex["AmmoCost"]] = "(lambda (design) 156)";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createUraniumAmmoComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "UraniumExplosives");
    comp->setDescription( "Most basic nuclear explosive");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["AmmoSize"]] = "(lambda (design) 4)";
    propertylist[propertyIndex["AmmoExplosiveness"]] = "(lambda (design) 1)";
    // Ammo cost is (cost/size) * size.
    // This is for one 'unit' of explosive - if a missile can fit two units,
    // the cost for the missile bay is twice the AmmoCost property
    propertylist[propertyIndex["AmmoCost"]] = "(lambda (design) 4)";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createAntiparticleAmmoComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "AntiparticleExplosives");
    comp->setDescription( "An extremely expensive but hugely explosive particle and anit-particle explosive");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["AmmoSize"]] = "(lambda (design) 0.8)";
    propertylist[propertyIndex["AmmoExplosiveness"]] = "(lambda (design) 16)";
    // Ammo cost is (cost/size) * size.
    // This is for one 'unit' of explosive - if a missile can fit two units,
    // the cost for the missile bay is twice the AmmoCost property
    propertylist[propertyIndex["AmmoCost"]] = "(lambda (design) 64)";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createAntimatterAmmoComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "AntimatterExplosives");
    comp->setDescription( "An even more extremely expensive but insanely explosive antimatter-matter explosive");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["AmmoSize"]] = "(lambda (design) 0.8)";
    propertylist[propertyIndex["AmmoExplosiveness"]] = "(lambda (design) 16)";
    // Ammo cost is (cost/size) * size.
    // This is for one 'unit' of explosive - if a missile can fit two units,
    // the cost for the missile bay is twice the AmmoCost property
    propertylist[propertyIndex["AmmoCost"]] = "(lambda (design) 64)";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createThoriumAmmoComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "ThoriumExplosives");
    comp->setDescription( "A significantly cheaper but less explosive nuclear explosive");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["AmmoSize"]] = "(lambda (design) 4)";
    propertylist[propertyIndex["AmmoExplosiveness"]] = "(lambda (design) 0.5)";
    // Ammo cost is (cost/size) * size.
    // This is for one 'unit' of explosive - if a missile can fit two units,
    // the cost for the missile bay is twice the AmmoCost property
    propertylist[propertyIndex["AmmoCost"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-ammo"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createAlphaMissileBayComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "AlphaMissileBay");
    comp->setDescription( "An alpha missile bay, capable of firing one alpha missile per combat round");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
                "(if (< (designType.AmmoSize design) 4) "
                    "(cons #t \"\") "
                    "(cons #f \"Explosive is too large for alpha missiles\")) "
                "(cons #f \"Missiles can only handle one type of explosive\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["MissileSize"]] = "(lambda (design) 3)";
    // To get the missile's firepower, we multiply the explosiveness of
    // the material by how much material can fit in the warhead.
    // How much material can fit in the warhead is the missile size divided
    // by the ammo size.
    propertylist[propertyIndex["MissileFirepower"]] = "(lambda (design) "
        "(* (floor (/ 3 (designType.AmmoSize design))) (designType.AmmoExplosiveness design)))";
    // Missile bay Cost is ammo cost times how much material can fit in the warhead
    propertylist[propertyIndex["MissileCost"]] = "(lambda (design) "
        "(* (floor (/ 3 (designType.AmmoSize design))) (designType.AmmoCost design)))";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-baytypes"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createBetaMissileBayComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "BetaMissileBay");
    comp->setDescription( "A beta missile bay, capable of firing one beta missile per combat round");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
                "(if (< (designType.AmmoSize design) 7) "
                    "(cons #t \"\") "
                    "(cons #f \"Explosive is too large for beta missiles\")) "
                "(cons #f \"Missiles can only handle one type of explosive\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["MissileSize"]] = "(lambda (design) 6)";
    // To get the missile's firepower, we multiply the explosiveness of
    // the material by how much material can fit in the warhead.
    // How much material can fit in the warhead is the missile size divided
    // by the ammo size.
    propertylist[propertyIndex["MissileFirepower"]] = "(lambda (design) "
        "(* (floor (/ 6 (designType.AmmoSize design))) (designType.AmmoExplosiveness design)))";
    // Missile bay Cost is ammo cost times how much material can fit in the warhead
    propertylist[propertyIndex["MissileCost"]] = "(lambda (design) "
        "(* (floor (/ 6 (designType.AmmoSize design))) (designType.AmmoCost design)))";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-baytypes"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createGammaMissileBayComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "GammaMissileBay");
    comp->setDescription( "A gamma missile bay, capable of firing one gamma missile per combat round");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
                "(if (< (designType.AmmoSize design) 9) "
                    "(cons #t \"\") "
                    "(cons #f \"Explosive is too large for gamma missiles\")) "
                "(cons #f \"Missiles can only handle one type of explosive\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["MissileSize"]] = "(lambda (design) 8)";
    // To get the missile's firepower, we multiply the explosiveness of
    // the material by how much material can fit in the warhead.
    // How much material can fit in the warhead is the missile size divided
    // by the ammo size.
    propertylist[propertyIndex["MissileFirepower"]] = "(lambda (design) "
        "(* (floor (/ 8 (designType.AmmoSize design))) (designType.AmmoExplosiveness design)))";
    // Missile bay Cost is ammo cost times how much material can fit in the warhead
    propertylist[propertyIndex["MissileCost"]] = "(lambda (design) "
        "(* (floor (/ 8 (designType.AmmoSize design))) (designType.AmmoCost design)))";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-baytypes"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createDeltaMissileBayComp()
{
    std::map<unsigned int, std::string> propertylist;
    DesignStore *ds = Game::getGame()->getDesignStore();
    Component* comp = new Component();

    comp->setCategoryId(1);
    comp->setName( "DeltaMissileBay");
    comp->setDescription( "A delta missile bay, capable of firing one delta missile per combat round");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType.num-ammo design) 1) "
                "(if (< (designType.AmmoSize design) 13) "
                    "(cons #t \"\") "
                    "(cons #f \"Explosive is too large for gamma missiles\")) "
                "(cons #f \"Missiles can only handle one type of explosive\")))");
    propertylist[propertyIndex["Speed"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["BuildTime"]] = "(lambda (design) 1)";
    propertylist[propertyIndex["Colonise"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["Armor"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["MissileSize"]] = "(lambda (design) 12)";
    // To get the missile's firepower, we multiply the explosiveness of
    // the material by how much material can fit in the warhead.
    // How much material can fit in the warhead is the missile size divided
    // by the ammo size.
    propertylist[propertyIndex["MissileFirepower"]] = "(lambda (design) "
        "(* (floor (/ 12 (designType.AmmoSize design))) (designType.AmmoExplosiveness design)))";
    // Missile bay Cost is ammo cost times how much material can fit in the warhead
    propertylist[propertyIndex["MissileCost"]] = "(lambda (design) "
        "(* (floor (/ 12 (designType.AmmoSize design))) (designType.AmmoCost design)))";
    propertylist[propertyIndex["StartingHitPoints"]] = "(lambda (design) 0)";
    propertylist[propertyIndex["num-baytypes"]] = "(lambda (design) 1)";
    comp->setPropertyList(propertylist);
    ds->addComponent(comp);

    return;
}


void MTSec::createProperties()
{
    Logger::getLogger()->debug( "Enter MTSec::createProperties");

    createSpeedProp();
    createAmmoCostProp();
    createAmmoExplosivenessProp();
    createAmmoSizeProp();
    createFirepowerProp();
    createMissileCostProp();
    createMissileFirepowerProp();
    createMissileSizeProp();
    createHitPointsProp();
    createHPProp();
    createBuildTimeProp();
    createArmorProp();
    createColoniseProp();
    createNumAmmoProp();
    createNumBayTypesProp();
    createNumHullsProp();

    Logger::getLogger()->debug( "Exit MTSec::createProperties");

    return;
}


void MTSec::createComponents()
{
    Logger::getLogger()->debug( "Enter MTSec::createComponents");

    createScoutHullComp();
    createBattleScoutHullComp();
    createCerium3AmmoComp();
    createCerium6AmmoComp();
    createCerium12AmmoComp();
    createUraniumAmmoComp();
    createAntiparticleAmmoComp();
    createAntimatterAmmoComp();
    createThoriumAmmoComp();
    createAlphaMissileBayComp();
    createBetaMissileBayComp();
    createGammaMissileBayComp();
    createDeltaMissileBayComp();

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
IGObject* MTSec::createAlphaCentauriSystem( IGObject* mw_galaxy)
{
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    ResourceManager* resman = game->getResourceManager();
    IGObject*      ac = game->getObjectManager()->createNewObject();
    IGObject*      acprime = game->getObjectManager()->createNewObject();

    uint32_t obT_Star_System = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Star System");
    uint32_t obT_Planet = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Planet");

    
    ac->setType(obT_Star_System);
    ac->setSize(800000ll);
    ac->setName("Alpha Centauri System");
    ac->setPosition(Vector3d(-1500000000ll, 1500000000ll, 0ll));
    ac->setVelocity(Vector3d(0ll, 0ll, 0ll));
    ac->addToParent(mw_galaxy->getID());
    obman->addObject(ac);

    acprime->setType(obT_Planet);
    acprime->setSize(2);
    acprime->setName("Alpha Centauri Prime");
    acprime->setPosition(ac->getPosition() + Vector3d(-6300ll, 78245ll, 0ll));
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ((Planet*)(acprime->getObjectData()))->setResources(ress);
    
    OrderQueue *planetoq = new OrderQueue();
    planetoq->setQueueId(acprime->getID());
    planetoq->addOwner(0);
    game->getOrderManager()->addOrderQueue(planetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(acprime->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(planetoq->getQueueId());
    ((Planet*)(acprime->getObjectData()))->setDefaultOrderTypes();
    
    acprime->addToParent(ac->getID());
    obman->addObject(acprime);

    return ac;
}


// Create the Sirius star system
IGObject* MTSec::createSiriusSystem( IGObject* mw_galaxy)
{
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    ResourceManager* resman = game->getResourceManager();
    IGObject*      sirius = game->getObjectManager()->createNewObject();
    IGObject*      s1 = game->getObjectManager()->createNewObject();
    
    uint32_t obT_Star_System = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Star System");
    uint32_t obT_Planet = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Planet");

    sirius->setType(obT_Star_System);
    sirius->setSize(2000000ll);
    sirius->setName("Sirius System");
    sirius->setPosition(Vector3d(-250000000ll, -3800000000ll, 0ll));
    sirius->setVelocity(Vector3d(0ll, 0ll, 0ll));
    sirius->addToParent(mw_galaxy->getID());
    obman->addObject(sirius);

    s1->setType(obT_Planet);
    s1->setSize(2);
    s1->setName("Sirius 1");
    s1->setPosition(sirius->getPosition() + Vector3d(45925ll, -34262ll, 0ll));
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ((Planet*)(s1->getObjectData()))->setResources(ress);
    
    OrderQueue *planetoq = new OrderQueue();
    planetoq->setQueueId(s1->getID());
    planetoq->addOwner(0);
    game->getOrderManager()->addOrderQueue(planetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(s1->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(planetoq->getQueueId());
    ((Planet*)(s1->getObjectData()))->setDefaultOrderTypes();
    
    s1->addToParent(sirius->getID());
    obman->addObject(s1);

    return sirius;
}


// Returns a random number between 1 and 'max'
static unsigned int myRandom( unsigned int  max)
{
    return Game::getGame()->getRandom()->getInRange(1U, max);
}


// Create a random star system
IGObject* MTSec::createStarSystem( IGObject* mw_galaxy)
{
    Logger::getLogger()->debug( "Entering MTSec::createStarSystem");
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    IGObject*      star = game->getObjectManager()->createNewObject();
    unsigned int   nplanets = 0;
    std::ostringstream     formatter;
    
    uint32_t obT_Star_System = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Star System");
    uint32_t obT_Planet = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Planet");

    star->setType( obT_Star_System);
    star->setSize(1400000ll);
    unsigned int   thx = myRandom(45);
    star->setName(systemNames[thx-1]);
    star->setPosition( Vector3d( myRandom(8000) * 1000000ll - 4000000000ll,
                                 myRandom(8000) * 1000000ll - 4000000000ll,
                                 0ll));
    star->setVelocity( Vector3d( 0ll, 0ll, 0ll));
    star->addToParent( mw_galaxy->getID());
    obman->addObject( star);

    // Create a variable number of planets for each star system
    while ( nplanets < 5 && myRandom(10) < 6) {
        IGObject*  planet = game->getObjectManager()->createNewObject();
        formatter.str("");
        formatter << star->getName() << " " << nplanets;

        planet->setType( obT_Planet);
        planet->setSize( 2);
        planet->setName( formatter.str().c_str());
        planet->setPosition( star->getPosition() + Vector3d( nplanets * 40000ll,
                                                             nplanets * -35000ll,
                                                             0ll));

        ResourceManager* resman = game->getResourceManager();
        std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
        ress[resman->getResourceDescription("Home Planet")->getResourceType()] =  std::pair<uint32_t, uint32_t>(1, 0);
        ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
        ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
        ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
        ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
        ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
        ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
        ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
        ((Planet*)(planet->getObjectData()))->setResources(ress);
        
        OrderQueue *planetoq = new OrderQueue();
        planetoq->setQueueId(planet->getID());
        planetoq->addOwner(0);
        game->getOrderManager()->addOrderQueue(planetoq);
        OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(planet->getObjectData()->getParameterByType(obpT_Order_Queue));
        oqop->setQueueId(planetoq->getQueueId());
        ((Planet*)(planet->getObjectData()))->setDefaultOrderTypes();

        planet->addToParent( star->getID());
        obman->addObject( planet);
        nplanets++;
    }

    Logger::getLogger()->debug( "Exiting MTSec::createStarSystem");
    return star;
}


// Create the Sol star system
IGObject* MTSec::createSolSystem( IGObject *mw_galaxy)
{
    Game*          game = Game::getGame();
    ObjectManager* obman = game->getObjectManager();
    ResourceManager* resman = game->getResourceManager();
    IGObject*      sol = game->getObjectManager()->createNewObject();
    IGObject*      earth = game->getObjectManager()->createNewObject();
    IGObject*      venus = game->getObjectManager()->createNewObject();
    IGObject*      mars = game->getObjectManager()->createNewObject();
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    
    uint32_t obT_Star_System = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Star System");
    uint32_t obT_Planet = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Planet");
    
    sol->setType(obT_Star_System);
    sol->setSize(1400000ll);
    sol->setName("Sol/Terra System");
    sol->setPosition(Vector3d(3000000000ll, 2000000000ll, 0ll));
    sol->setVelocity(Vector3d(0ll, 0ll, 0ll));
    sol->addToParent(mw_galaxy->getID());
    obman->addObject(sol);

    earth->setType(obT_Planet);
    earth->setSize(2);
    earth->setName("Earth/Terra");
    earth->setPosition(sol->getPosition() + Vector3d(14960ll, 0ll, 0ll));
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ((Planet*)(earth->getObjectData()))->setResources(ress);
    
    OrderQueue *planetoq = new OrderQueue();
    planetoq->setQueueId(earth->getID());
    planetoq->addOwner(0);
    game->getOrderManager()->addOrderQueue(planetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(earth->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(planetoq->getQueueId());
    ((Planet*)(earth->getObjectData()))->setDefaultOrderTypes();
    
    earth->addToParent(sol->getID());
    obman->addObject(earth);

    ress.clear();

    venus->setType(obT_Planet);
    venus->setSize(2);
    venus->setName("Venus");
    venus->setPosition(sol->getPosition() + Vector3d(0ll, 10800ll, 0ll));
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ((Planet*)(venus->getObjectData()))->setResources(ress);
    
    planetoq = new OrderQueue();
    planetoq->setQueueId(venus->getID());
    planetoq->addOwner(0);
    game->getOrderManager()->addOrderQueue(planetoq);
    oqop = static_cast<OrderQueueObjectParam*>(venus->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(planetoq->getQueueId());
    ((Planet*)(venus->getObjectData()))->setDefaultOrderTypes();
    
    venus->addToParent(sol->getID());
    obman->addObject(venus);

    ress.clear();

    mars->setType(obT_Planet);
    mars->setSize(1);
    mars->setName("Mars");
    mars->setPosition(sol->getPosition() + Vector3d(-22790ll, 0ll, 0ll));
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ((Planet*)(mars->getObjectData()))->setResources(ress);
    
    planetoq = new OrderQueue();
    planetoq->setQueueId(mars->getID());
    planetoq->addOwner(0);
    game->getOrderManager()->addOrderQueue(planetoq);
    oqop = static_cast<OrderQueueObjectParam*>(mars->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(planetoq->getQueueId());
    ((Planet*)(mars->getObjectData()))->setDefaultOrderTypes();
    
    mars->addToParent(sol->getID());
    obman->addObject(mars);

    return sol;
}


void MTSec::createResources(){
  ResourceManager* resman = Game::getGame()->getResourceManager();
  
  ResourceDescription* res = new ResourceDescription();
  res->setNameSingular("Ship part");
  res->setNamePlural("Ship parts");
  res->setUnitSingular("part");
  res->setUnitPlural("parts");
  res->setDescription("Ships parts that can be used to create ships");
  res->setMass(0);
  res->setVolume(0);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Home Planet");
  res->setNamePlural("Home Planets");
  res->setUnitSingular("unit");
  res->setUnitPlural("units");
  res->setDescription("The home planet for a race.");
  res->setMass(0);
  res->setVolume(0);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Uranium");
  res->setNamePlural("Uranium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Uranium Ore.");
  res->setMass(1);
  res->setVolume(4);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Thorium");
  res->setNamePlural("Thorium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Thorium Ore.");
  res->setMass(1);
  res->setVolume(4);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Cerium");
  res->setNamePlural("Cerium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Cerium Ore.");
  res->setMass(1);
  res->setVolume(3);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Enriched Uranium");
  res->setNamePlural("Enriched Uranium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Enriched Uranium Ore.");
  res->setMass(1);
  res->setVolume(2);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Massivium");
  res->setNamePlural("Massivium");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Raw Massivium Ore.");
  res->setMass(1);
  res->setVolume(12);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Antiparticle");
  res->setNamePlural("Antiparticle");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Antiparticles in magnetic containment.");
  res->setMass(1);
  res->setVolume(1);
  resman->addResourceDescription(res);
  
  res = new ResourceDescription();
  res->setNameSingular("Antimatter");
  res->setNamePlural("Antimatter");
  res->setUnitSingular("kt");
  res->setUnitPlural("kt");
  res->setDescription("Antimatter in magnetic containment.");
  res->setMass(1);
  res->setVolume(1);
  resman->addResourceDescription(res);
  
}


void MTSec::createGame()
{
    Logger::getLogger()->debug( "Enter MTSec::createGame");
    Game*        game = Game::getGame();
    DesignStore* ds = game->getDesignStore();
    unsigned int counter;
    Category*    cat = new Category();
    cat->setName("Ships");
    cat->setDescription("The Ship design and component category");
    ds->addCategory(cat);
    assert(cat->getCategoryId() == 1);

    createTechTree();
    createResources();

    ObjectManager* obman = game->getObjectManager();

    uint32_t obT_Universe = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Universe");
    uint32_t obT_Galaxy = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Galaxy");
    
    IGObject* universe = game->getObjectManager()->createNewObject();
    universe->setType(obT_Universe);
    universe->setSize(100000000000ll);
    universe->setName("The Universe");
    universe->setPosition(Vector3d(0ll, 0ll, 0ll));
    universe->setVelocity(Vector3d(0ll, 0ll, 0ll));
    obman->addObject(universe);

    //add contained objects
    IGObject *mw_galaxy = game->getObjectManager()->createNewObject();
    mw_galaxy->setType(obT_Galaxy);
    mw_galaxy->setSize(10000000000ll);
    mw_galaxy->setName("Milky Way Galaxy");
    mw_galaxy->setPosition(Vector3d(0ll, -6000ll, 0ll));
    mw_galaxy->setVelocity(Vector3d(0ll, 0ll, 0ll));
    mw_galaxy->addToParent(universe->getID());
    obman->addObject(mw_galaxy);

    // Some initial star systems...
    createSolSystem( mw_galaxy);
    createAlphaCentauriSystem( mw_galaxy);
    createSiriusSystem( mw_galaxy);
    for ( counter = 0; counter < 45; counter++) {
        createStarSystem( mw_galaxy);
    }

    propertyIndex.clear();
    
    Logger::getLogger()->debug( "Exit MTSec::createGame");

    return;
}


void MTSec::startGame()
{
  Game::getGame()->setTurnLength(600);
}



bool MTSec::onAddPlayer(Player* player)
{
  return true;
}


Design* MTSec::createScoutDesign( Player* owner)
{
    Game *game = Game::getGame();
    Design* scout = new Design();
    std::map<unsigned int, unsigned int> componentList;

    scout->setCategoryId(1);
    scout->setName( "Scout");
    scout->setDescription("Scout ship");
    scout->setOwner( owner->getID());
    componentList[1] = 1;
    scout->setComponents(componentList);
    game->getDesignStore()->addDesign(scout);

    return scout;
}


Design* MTSec::createBattleScoutDesign( Player* owner)
{
    Logger::getLogger()->debug( "Enter MTSec::createBattleScoutDesign");
    Game *game = Game::getGame();
    Design* scout = new Design();
    std::map<unsigned int, unsigned int> componentList;

    scout->setCategoryId(1);
    scout->setName( "BattleScout");
    scout->setDescription("Battle Scout ship");
    scout->setOwner( owner->getID());
    componentList[2] = 1;
    componentList[10] = 1;
    componentList[3] = 1;
    scout->setComponents(componentList);
    game->getDesignStore()->addDesign(scout);

    Logger::getLogger()->debug( "Exit MTSec::createBattleScoutDesign");
    return scout;
}


IGObject* MTSec::createEmptyFleet( Player*     owner,
                                   IGObject*   star,
                                   std::string fleetName)
{
    Game *game = Game::getGame();
    IGObject *fleet = game->getObjectManager()->createNewObject();
        
    fleet->setType(game->getObjectDataManager()->getObjectTypeByName("Fleet"));
    
    Vector3d  offset = Vector3d( ( long long) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 ( long long) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 /*(long long)((rand() % 10000) - 5000)*/ 0);
    
    fleet->setSize( 2);
    fleet->setName( fleetName.c_str());
    ((OwnedObject*)(fleet->getObjectData()))->setOwner(owner->getID());

    // Place the fleet in orbit around the given star
    fleet->setPosition( star->getPosition() + offset);
    fleet->setVelocity( Vector3d(0LL, 0ll, 0ll));
    
    OrderQueue *fleetoq = new OrderQueue();
    fleetoq->setQueueId(fleet->getID());
    fleetoq->addOwner(owner->getID());
    game->getOrderManager()->addOrderQueue(fleetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(fleet->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(fleetoq->getQueueId());
    ((Fleet*)(fleet->getObjectData()))->setDefaultOrderTypes();

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
void MTSec::makeNewPlayerFleet( Player* player, IGObject* star)
{
    Logger::getLogger()->debug( "Enter MTSec::makeNewPlayerFleet");
    Game *game = Game::getGame();
    std::string fleetName = player->getName().substr( 0,11) + " Fleet 1";
    IGObject*   fleet = createEmptyFleet( player, star, fleetName);

    Design* scout = createBattleScoutDesign( player);

    // Start this fleet off with two battle scout ships
    ((Fleet*)(fleet->getObjectData()))->addShips( scout->getDesignId(), 2);
    scout->addUnderConstruction(2);
    scout->addComplete(2);
    game->getDesignStore()->designCountsUpdated( scout);

    game->getObjectManager()->addObject( fleet);

    Logger::getLogger()->debug( "Exit MTSec::makeNewPlayerFleet");
    return;
}


// Create a new player's home planet, orbiting around
// the given star.
IGObject* MTSec::makePlayerHomePlanet( Player* player, IGObject* star)
{
    Logger::getLogger()->debug( "Enter MTSec::makePlayerHomePlanet");
    Game *    game = Game::getGame();
    IGObject* planet = game->getObjectManager()->createNewObject();
    
    std::string planetName = player->getName() + " Planet";

    uint32_t obT_Planet = Game::getGame()->getObjectDataManager()->getObjectTypeByName("Planet");
    
    planet->setType( obT_Planet);
    Vector3d  offset = Vector3d( ( long long) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 ( long long) ( game->getRandom()->getInRange(0, 10000) - 5000),
                                 /*(long long)((rand() % 10000) - 5000)*/ 0);
    planet->setSize( 2);
    planet->setName( planetName.c_str());
    ((OwnedObject*)(planet->getObjectData()))->setOwner(player->getID());
    planet->setPosition( star->getPosition() + offset);
    planet->setVelocity( Vector3d( 0LL, 0ll, 0ll));

    ResourceManager* resman = game->getResourceManager();
    std::map<uint32_t, std::pair<uint32_t, uint32_t> > ress;
    ress[resman->getResourceDescription("Home Planet")->getResourceType()] =  std::pair<uint32_t, uint32_t>(1, 0);
    ress[resman->getResourceDescription("Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Thorium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(20, 100));
    ress[resman->getResourceDescription("Massivium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(10, 100));
    ress[resman->getResourceDescription("Enriched Uranium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(7, 70));
    ress[resman->getResourceDescription("Cerium")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(5, 50));
    ress[resman->getResourceDescription("Antiparticle")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 30));
    ress[resman->getResourceDescription("Antimatter")->getResourceType()] = std::pair<uint32_t, uint32_t>(0, game->getRandom()->getInRange(1, 15));
    ((Planet*)(planet->getObjectData()))->setResources(ress);
    
    OrderQueue *planetoq = new OrderQueue();
    planetoq->setQueueId(planet->getID());
    planetoq->addOwner(0);
    game->getOrderManager()->addOrderQueue(planetoq);
    OrderQueueObjectParam* oqop = static_cast<OrderQueueObjectParam*>(planet->getObjectData()->getParameterByType(obpT_Order_Queue));
    oqop->setQueueId(planetoq->getQueueId());
    ((Planet*)(planet->getObjectData()))->setDefaultOrderTypes();

    planet->addToParent( star->getID());
    game->getObjectManager()->addObject(planet);

    Logger::getLogger()->debug( "Exit MTSec::makePlayerHomePlanet");
    return planet;
}


// Create a 'home star system' for a new player
//
// These 'home systems' always consist of exactly one planet.
//
IGObject* MTSec::makeNewPlayerStarSystem( Player* player)
{
    Logger::getLogger()->debug( "Enter MTSec::makeNewPlayerStarSystem");
    Game *    game = Game::getGame();
    IGObject* star = game->getObjectManager()->createNewObject();
    
    std::string starName = player->getName() + " Star System";

    star->setType(game->getObjectDataManager()->getObjectTypeByName("Star System"));
    Vector3d  location = Vector3d( ( long long) ( ( game->getRandom()->getInRange(0, 1000) - 500) * 10000000),
                                   ( long long) ( ( game->getRandom()->getInRange(0, 1000) - 500) * 10000000),
                                   /*(long long)(((rand()%1000)-500)*10000000)*/ 0);
    star->setSize( 2000000ll);
    star->setName( starName.c_str());
    star->setPosition( location);
    star->setVelocity( Vector3d( 0ll, 0ll, 0ll));

    star->addToParent(1);
    game->getObjectManager()->addObject( star);

    makePlayerHomePlanet( player, star);

    Logger::getLogger()->debug( "Exit MTSec::makeNewPlayerStarSystem");
    return star;
}


// This routine sets a new player's initial tech level.
// New players can make any hull, see all stars.  This
// doesn't leave anything for them to research.
void MTSec::setNewPlayerTech( Player* player)
{
    Logger::getLogger()->debug( "Enter MTSec::setNewPlayerTech");
    Game *game = Game::getGame();

    player->setVisibleObjects( game->getObjectManager()->getAllIds());

    for(uint32_t itcurr = 1; itcurr <= compMax; ++itcurr){
      player->addVisibleComponent(itcurr);
      player->addUsableComponent(itcurr);
    }

    Logger::getLogger()->debug( "Exit MTSec::setNewPlayerTech");
    return;
}


// New players start with their own star systems, and an
// initial fleet consisting of two scout ships.
//
void MTSec::onPlayerAdded(Player* player)
{
    Logger::getLogger()->debug( "Enter MTSec::onPlayerAdded");

    IGObject* star = makeNewPlayerStarSystem( player);

    setNewPlayerTech( player);
    makeNewPlayerFleet( player, star);
    Game::getGame()->getPlayerManager()->updatePlayer( player->getID());

    Logger::getLogger()->debug( "Exit MTSec::onPlayerAdded");
    return;
}
