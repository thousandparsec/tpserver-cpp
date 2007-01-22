/*  Tp Guile Interpreter class
 *
 *  Copyright (C) 2005,2006  Lee Begg and the Thousand Parsec Project
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

#include <libguile.h>
#include <guile/gh.h>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifndef DATADIR
#define DATADIR "/usr/local/share"
#endif
#endif

#include <tpserver/design.h>
#include <tpserver/logging.h>
#include <tpserver/designstore.h>
#include <tpserver/game.h>
#include <tpserver/component.h>
#include <tpserver/property.h>
#include <tpserver/propertyvalue.h>

#include "tpguile.h"

extern "C" {
  bool tp_init(){
    return Game::getGame()->setTpScheme(new TpGuile());
  }
}

TpGuile::~TpGuile(){
}


// Defines the property-designType-set! procedure in guile.
void TpGuile::definePropertyDesignTypeSet()
{
    scm_c_eval_string("(define property-designType-set! (lambda (design id val) (designType-set! design (- id 1) val)))");
}


//
// Create the design structure in Guile.
//
// This associates the 'design' name to the new design structure, effectively
// making this the 'current' design
//
// This is done in two stages: first the vtable structure is created,
// which defines the fields that the final structure will need.
// Then the real structure is created with the fields described in the
// vtable structure.
//
// Enough fields are allocated in this structure to store values
// for every property in the designstore.  One extra is allocated
// at index 0 to store the number of components in the design.
//
void TpGuile::defineDesignType( Design * d)
{
    DesignStore *      ds = Game::getGame()->getDesignStore();
    SCM                temp;
    int                counter;
    std::ostringstream formater;

    // Create the vtable, with name <design>DesignType
    formater.str( "");
    formater << "(define " << d->getName() << "DesignType (make-design-vtable \"";
    for ( counter = ds->getMaxPropertyId(); counter >= 0; counter--) {
        formater << "pw";
    }
    formater << "pw\"))";

    temp = scm_c_eval_string( formater.str().c_str());

    // Now create the design structure itself, with name 'design'
    formater.str( "");
    formater << "(define design (make-struct " << d->getName() << "DesignType 0))";
    temp = scm_c_eval_string( formater.str().c_str());

    return;
}


// Defines a property accessor function for each property in the
// current game's design store.
void TpGuile::definePropertyAccessors()
{
    DesignStore *          ds = Game::getGame()->getDesignStore();
    SCM                    temp;
    std::set<unsigned int> propids = ds->getPropertyIds();
    std::ostringstream     formater;

    formater.str( "(define (designType._num-components ship) (struct-ref ship 0))");
    temp = scm_c_eval_string(formater.str().c_str());

    for ( std::set<unsigned int>::iterator propit = propids.begin();
          propit != propids.end(); ++propit) {        // for each property type...
        Property* p = ds->getProperty( *propit);

        if ( p != NULL) {
            formater.str("");
            formater << "(define (designType." << p->getName()
                     << " ship) (struct-ref ship "
                     << ( p->getPropertyId() + 1) << "))";
            temp = scm_c_eval_string(formater.str().c_str());
        }
    }

    propids.clear();
}


// Set the property value for the current design in Guile
void TpGuile::setDesignPropertyValue( const PropertyValue & pv)
{
    scm_call_3( scm_variable_ref( scm_c_lookup( "struct-set!")),
                scm_variable_ref( scm_c_lookup( "design")),
                scm_int2num( pv.getPropertyId() + 1),
                scm_double2num( pv.getValue()));
}


// Set the number of components for the current design in Guile
void TpGuile::setDesignComponentCount( const unsigned int count)
{
    scm_call_3( scm_variable_ref( scm_c_lookup( "struct-set!")),
                scm_variable_ref( scm_c_lookup( "design")),
                scm_int2num( 0),
                scm_int2num( count));
}


std::map<unsigned int, std::map<unsigned int, std::list<std::string> > > *
TpGuile::createPropertyRankingMap( std::map<unsigned int, unsigned int> & complist)
{
    DesignStore *ds = Game::getGame()->getDesignStore();
    std::map<unsigned int, std::map<unsigned int, std::list<std::string> > > * propranking =
        new std::map<unsigned int, std::map<unsigned int, std::list<std::string> > >();

    for ( std::map<unsigned int, unsigned int>::iterator compit = complist.begin();
          compit != complist.end(); ++compit) {
        Component *c = ds->getComponent( compit->first);
        std::map<unsigned int, std::string> pilist = c->getPropertyList();

        for ( std::map<unsigned int, std::string>::iterator piit = pilist.begin();
              piit != pilist.end(); ++piit) {
            Property* p = ds->getProperty( piit->first);
            for ( unsigned int i = 0; i < compit->second; i++) {
                (*propranking)[p->getRank()][p->getPropertyId()].push_back(piit->second);
            }
        }
    }

    return propranking;
}


double TpGuile::evalCompProperty( std::string lambdaStr)
{
    double   result = 0.0;
    SCM      temp;

    temp = scm_c_eval_string( ( std::string("(") + lambdaStr + " design)").c_str());
    if ( ! SCM_NUMBERP( temp)) {
        Logger::getLogger()->warning( "Guile: Return not a number");
    }
    else {
        result = scm_num2double( temp, 1, "evalCompProperty");
    }

    return result;
}


//
// Calculate the value of a property for a design.
//
// OK, we're given a property and a list of scheme lambda strings, each
// of which defines a value for that property for one component of the
// design.
// First the list of component property strings is traversed, with guile
// invoked on each one to interpret it and return the result as a double.
// This list of doubles is then passed in to the property's display function,
// which chews on it and (apparently) returns the name of the property
// and a single value.
// The resulting name, value, and property ID are returned in a PropertyValue
// object as the return value of the function.
PropertyValue TpGuile::getPropertyValue( Property * p,
                                         std::list<std::string> & compPropStrList)
{
    SCM           temp;
    PropertyValue propval;
    SCM           s_total_list = SCM_EOL;
    size_t        length;

    // Eval each lambda string in the list, and post the result
    // to the end of the s_total_list list.
    for ( std::list<std::string>::iterator lambdaIter = compPropStrList.begin();
          lambdaIter != compPropStrList.end(); ++lambdaIter) {
        SCM     s_element;
        SCM     s_element_list;

        s_element = scm_double2num( evalCompProperty(*lambdaIter));
        s_element_list = scm_list_1( s_element);
        s_total_list = scm_append( scm_list_2( s_total_list, s_element_list));
    }

    // Now pass this list to the property's propertyCalculate function
    // (a.k.a. it's Tpcl Display function) to aggregate the values together.
    temp = scm_call_2( scm_c_eval_string( p->getTpclDisplayFunction().c_str()),
                       scm_variable_ref( scm_c_lookup( "design")),
                       s_total_list);

    // Place the result of the call to propertyCalculate in a PropertyValue structure.
    if ( ! SCM_CONSP( temp) ||
         ! SCM_NUMBERP( SCM_CAR( temp)) ||
         ! SCM_STRINGP( SCM_CDR( temp))) {
        Logger::getLogger()->warning( "Guile: Return not a pair, or the wrong type in the pair");
    }
    else {
        propval.setPropertyId( p->getPropertyId());
        propval.setDisplayString( std::string( gh_scm2newstr( SCM_CDR( temp), &length)));
        propval.setValue( scm_num2double( SCM_CAR( temp), 1, "getPropertyValue"));
    }

    return propval;
}


// Calculate the property value for each component in the given design,
// returning them in a mapping from property ID to property value structure.
std::map<unsigned int, PropertyValue> *
      TpGuile::calculateDesignPropertyValues( Design* d)
{
  DesignStore *ds = Game::getGame()->getDesignStore();
  // This includes not only the component IDs for the design,
  // but also how many of each component are needed for the design.
  std::map<unsigned int, unsigned int> complist = d->getComponents();
  // map for holding set of property values contributed by each component
  // that is part of the design.  This is structured first by rank so
  // that all the properties of lower rank are calculated before any
  // property of a higher rank is.  Within each rank, properties are
  // tracked by property ID.  Finally, each property has a list of
  // lambda strings that define that property's value for each component
  // in the design (that has that property)
  std::map<unsigned int, std::map<unsigned int, std::list<std::string> > > *
               desCompPropMap = createPropertyRankingMap( complist);
  std::map<unsigned int, PropertyValue> *
               propertyvalues = new std::map<unsigned int, PropertyValue>();

  setDesignComponentCount( complist.size());
  for ( std::map<unsigned int, std::map<unsigned int, std::list<std::string> > >::iterator rankIter = desCompPropMap->begin();
        rankIter != desCompPropMap->end(); ++rankIter) {
      std::map<unsigned int, std::list<std::string> > pilist = rankIter->second;
      std::set<PropertyValue> localvalues;

      for ( std::map<unsigned int, std::list<std::string> >::iterator piit = pilist.begin();
            piit != pilist.end(); ++piit) {
          PropertyValue    propval = getPropertyValue( ds->getProperty( piit->first), piit->second);
          localvalues.insert(propval);
      }

      // Set the property values in Guile so they'll be available
      // for the next rank.  They also need to be recorded with
      // the C++ design object.
      for ( std::set<PropertyValue>::iterator pvit = localvalues.begin();
            pvit != localvalues.end(); ++pvit) {
          setDesignPropertyValue( *pvit);
          (*propertyvalues)[pvit->getPropertyId()] = *pvit;
      }
  }

  desCompPropMap->clear();
  delete desCompPropMap;

  return propertyvalues;
}


bool TpGuile::evalRequirementFtn( std::string function, std::string & why)
{
    bool        valid = false;
    std::string schStr = std::string( "(") + function + " design)";
    SCM         temp = scm_c_eval_string( schStr.c_str());
    size_t      length;

    if ( ! SCM_CONSP( temp) ||
         ! SCM_STRINGP( SCM_CDR( temp))) {
        Logger::getLogger()->warning( "Guile: (a) Return not a pair, or the wrong type in the pair");
    }
    else {
        valid = ! SCM_FALSEP( SCM_CAR( temp));
        why = gh_scm2newstr( SCM_CDR( temp), &length);
    }

    return valid;
}


bool TpGuile::canPropGoInDesign( unsigned int propertyID, std::string & why)
{
    DesignStore *  ds = Game::getGame()->getDesignStore();
    Property*      propertyp = ds->getProperty( propertyID);

    return evalRequirementFtn( propertyp->getTpclRequirementsFunction(), why);
}


bool TpGuile::canCompGoInDesign( unsigned int componentID, std::string & why)
{
    DesignStore *  ds = Game::getGame()->getDesignStore();
    Component*     componentp = ds->getComponent( componentID);

    return evalRequirementFtn( componentp->getTpclRequirementsFunction(), why);
}


// Cycles through all the components in the given design d, evaluating the
// requirements function for each one in the context of the current design.
// (which should be d)  Then cycles through all the property values in
// propertyvalues, doing the same thing - evaluating the property value's
// requirements function in the context of the current design.  If any
// component or property value's requirement function returns false, sets
// the validity of d to false and outputs a message that the design is
// invalid.
void TpGuile::validateDesign( Design* d,
                              std::map<unsigned int, PropertyValue> & propertyvalues)
{
    std::map<unsigned int, unsigned int> complist = d->getComponents();
    bool           valid = true;
    std::string    feedback = "";

    Logger::getLogger()->debug( "About to process requirement functions");

    for ( std::map<unsigned int, unsigned int>::iterator compit = complist.begin();
          compit != complist.end();
          ++compit) {
        std::string    why;

        valid &= canCompGoInDesign( compit->first, why);
        if ( why.length() > 0)
            feedback += why + " ";
    }

    for ( std::map<unsigned int, PropertyValue >::iterator rpiit = propertyvalues.begin();
          rpiit != propertyvalues.end();
          ++rpiit) {
        std::string    why;

        valid &= canPropGoInDesign( rpiit->first, why);
        if ( why.length() > 0)
            feedback += why + " ";
    }

    d->setValid( valid, feedback);

    if ( ! valid) {
        Logger::getLogger()->debug( "Design %s is not valid, reason: %s",
                                    d->getName().c_str(), feedback.c_str());
    }
}


//
// Main entry point for the class.
//
// Given a Design d, calculate the values of the properties of d,
// and set those values in d (using the setPropertyValues() method
// of d).
//
// Also, mark whether the given Design is valid (using the setValid()
// method of d).
//
void TpGuile::evalDesign( Design* d)
{
    std::map<unsigned int, PropertyValue> * propertyvalues;

    Logger::getLogger()->debug( "Starting design eval");
    // definePropertyDesignTypeSet();
    definePropertyAccessors();
    defineDesignType( d);

    propertyvalues = calculateDesignPropertyValues( d);
    d->setPropertyValues( *propertyvalues);

    // now check if the design is valid
    validateDesign( d, *propertyvalues);

    delete propertyvalues;
    Logger::getLogger()->debug( "Eval'ed design");

    return;
}


TpGuile::TpGuile() {
    std::string  designTypeDefFile = "guile.scm";

    scm_init_guile();
    scm_c_eval_string("(set! %load-path (cons \"" DATADIR "/tpserver/tpscheme/guile/\" (cons \"modules/tpcl/guile/\" (cons \"../modules/tpcl/guile/\" %load-path ))))");
    scm_c_primitive_load_path( designTypeDefFile.c_str());
}
