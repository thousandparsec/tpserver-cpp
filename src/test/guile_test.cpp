//
// guile-test.cpp
//
// Tests for the guile functions.
//

#include "config.h"

#include <iostream>
#include <sstream>

#ifdef HAVE_GUILE
#include "tpguile.h"

#include <libguile.h>
#include <guile/gh.h>
#endif
#ifdef HAVE_LIBMZSCHEME
#endif

#include "game.h"
#include "designstore.h"
#include "category.h"
#include "property.h"
#include "propertyvalue.h"
#include "component.h"
#include "player.h"
#include "playermanager.h"
#include "design.h"


static Category*    shipCat;
static Property*    speedProp;
static Component*   scoutHull;
static Design*      scoutDesign;


static Category* createShipCategory()
{
    Category * cat = new Category();

    cat->setName("Ships");
    cat->setDescription("The Ship design and component category");
    Game::getGame()->getDesignStore()->addCategory(cat);

    return cat;
}


static Property* createSpeedProperty( unsigned int catID)
{
    Property* prop = new Property();

    prop->setCategoryId( catID);
    prop->setRank(0);
    prop->setName("Speed");
    prop->setDisplayName("Speed");
    prop->setDescription("The number of units the ship can move each turn");
    prop->setTpclDisplayFunction("(lambda (design bits) (let ((n (apply + bits))) (cons n (string-append (number->string (/ n 1000000)) \" mega-units\")) ) )");
    prop->setTpclRequirementsFunction("(lambda (design) (cons #t \"\"))");
    Game::getGame()->getDesignStore()->addProperty(prop);

    return prop;
}


static Component* createScoutHull( unsigned int catID, unsigned int speedPropID)
{
    Component* comp = new Component();
    std::map<unsigned int, std::string> propertylist;

    comp->setCategoryId( catID);
    comp->setName("ScoutHull");
    comp->setDescription("The scout hull, fitted out with everything a scout needs");
    comp->setTpclRequirementsFunction(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
            "(cons #f \"This is a complete component, nothing else can be included\")))");
    propertylist[speedPropID] = "(lambda (design) 300000000)";
    comp->setPropertyList(propertylist);
    Game::getGame()->getDesignStore()->addComponent(comp);

    return comp;
}


static Design* createScoutDesign( unsigned int catID,
                                  unsigned int hullID)
{
    Design* scout = new Design();
    std::map<unsigned int, unsigned int> componentList;

    scout->setCategoryId( catID);
    scout->setName( "Scout");
    scout->setDescription( "Scout ship");
    componentList[hullID] = 1;
    scout->setComponents( componentList);

    // Game::getGame()->getDesignStore()->addDesign(scout);

    return scout;
}


static void test6( TpGuile* myguile)
{
    std::cout << "Test 6:    ";

    myguile->evalDesign( scoutDesign);

    if ( scoutDesign->isValid())
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    return;
}


static void test5( TpGuile* myguile)
{
    std::string     why;
    bool            result = true;

    // test 5A
    std::cout << "Test 5A:   ";
    result = myguile->evalRequirementFtn(
        "(lambda (design) "
            "(cons #t \"All the world is a stage\"))",
        why);
    if ( result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    // test 5B
    std::cout << "Test 5B:   ";
    result = myguile->evalRequirementFtn(
        "(lambda (design) "
            "(cons #f \"second tests are always scsi\"))",
        why);
    if ( ! result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    // test 5C
    std::cout << "Test 5C:   ";
    result = myguile->evalRequirementFtn(
        "(lambda (design) "
            "(if (> (designType.Speed design) 100) "
                "(cons #t \"The faster the better\")"
                "(cons #f \"Way too slow\")))",
        why);
    if ( result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    // test 5D
    std::cout << "Test 5D:   ";
    result = myguile->evalRequirementFtn(
        "(lambda (design) "
            "(if (< (designType.Speed design) 100) "
                "(cons #t \"The faster the better\")"
                "(cons #f \"Way too slow\")))",
        why);
    if ( ! result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    // The next two tests test the _num-components accessor
    // test 5E
    std::cout << "Test 5E:   ";
    result = myguile->evalRequirementFtn(
        "(lambda (design) "
            "(if (= (designType._num-components design) 1) "
            "(cons #t \"\") "
        "(cons #f \"This is a complete component, nothing else can be included\")))",
        why);
    if ( result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    // test 5F
    std::cout << "Test 5F:   ";
    result = myguile->evalRequirementFtn(
        "(lambda (design) "
            "(if (= (designType._num-components design) 2) "
            "(cons #t \"\") "
        "(cons #f \"This is a complete component, nothing else can be included\")))",
        why);
    if ( ! result)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    return;
}


static void test4( TpGuile* myguile)
{
    std::cout << "Test 4:    ";

    myguile->setDesignComponentCount( 1);
    myguile->definePropertyAccessors();

    std::cout << "PASS" << std::endl;

    return;
}


static void test3( TpGuile* myguile)
{
    PropertyValue   propval;

    std::cout << "Test 3:    ";

    propval.setPropertyId( speedProp->getPropertyId());
    propval.setDisplayString( std::string( "really really fast"));
    propval.setValue( 230000);

    myguile->setDesignPropertyValue( propval);

    std::cout << "PASS" << std::endl;

    return;
}


static void test2( TpGuile* myguile)
{
    std::cout << "Test 2:    ";

    shipCat = createShipCategory();
    speedProp = createSpeedProperty( shipCat->getCategoryId());
    scoutHull = createScoutHull( shipCat->getCategoryId(),
                                 speedProp->getPropertyId());
    scoutDesign = createScoutDesign( shipCat->getCategoryId(),
                                     scoutHull->getComponentId());

    myguile->defineDesignType( scoutDesign);

    std::cout << "PASS" << std::endl;

    return;
}


static void test1( TpGuile* myguile)
{
    std::ostringstream formater;
    double    input = 3000;
    double    output = 0;

    std::cout << "Test 1:    ";

    formater.str( "");
    formater << "(lambda (design) " << input << ")";
    output = myguile->evalCompProperty( formater.str());

    if ( input == output)
        std::cout << "PASS" << std::endl;
    else
        std::cout << "FAIL" << std::endl;

    return;
}


int main(int argc, char **argv)
{

#ifdef HAVE_GUILE
    TpGuile * myguile = TpGuile::getImplemention();

    test1( myguile);
    test2( myguile);
    test3( myguile);
    test4( myguile);
    test5( myguile);
    test6( myguile);


#endif       // HAVE_GUILE

}
