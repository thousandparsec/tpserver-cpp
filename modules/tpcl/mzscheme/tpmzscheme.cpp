/*  MzScheme Interpreter class
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

#include <sstream>
#include <algorithm>
#include <vector>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifndef DATADIR
#define DATADIR "/usr/local/share"
#endif
#endif

#ifdef HAVE_SCHEME_H
#include <scheme.h>
#endif

#ifdef HAVE_PLT_SCHEME_H
#include <plt/scheme.h>
#endif

#include <tpserver/design.h>
#include <tpserver/logging.h>
#include <tpserver/designstore.h>
#include <tpserver/game.h>
#include <tpserver/component.h>
#include <tpserver/property.h>
#include <tpserver/propertyvalue.h>

#include "tpmzscheme.h"

extern "C" {
  bool tp_init(){
    return Game::getGame()->setTpScheme(new TpMzScheme());
  }
}


TpMzScheme::~TpMzScheme(){
}

void TpMzScheme::evalDesign(Design* d){

  DesignStore *ds = Game::getGame()->getDesignStore();
  
  if (scheme_setjmp(scheme_error_buf)) {
    Logger::getLogger()->warning("MzScheme Error");
  } else {
    Scheme_Object* temp;

    std::ostringstream formater;
   
    formater.str("");
    formater << "(define-values (struct:designType make-designType designType? designType-ref designType-set!)(make-design-type "
	     << ds->getMaxPropertyId() << "))";
    temp = scheme_eval_string(formater.str().c_str(), env);
    
    temp = scheme_eval_string("(define property-designType-set! (lambda (design id val) (designType-set! design (- id 1) val)))", env);

    std::set<unsigned int> propids = ds->getPropertyIds();
    for(std::set<unsigned int>::iterator propit = propids.begin();
	propit != propids.end(); ++propit){
      // for each property type
      Property* p = ds->getProperty(*propit);
      if(p != NULL){
	formater.str("");
	formater << "(define designType." << p->getName() 
		 << " (make-property-accessor designType-ref "
		 << p->getPropertyId() << " \"" << p->getName() 
		 << "\" ))";
	temp = scheme_eval_string(formater.str().c_str(), env);
	
      }
    }
    propids.clear();

        std::map<unsigned int, unsigned int> complist = d->getComponents();

    temp = scheme_eval_string("(define design (make-designType))", env);

        std::map<unsigned int, std::map<unsigned int, std::list<std::string> > > propranking;
        for(std::map<unsigned int, unsigned int>::iterator compit = complist.begin();
                compit != complist.end(); ++compit){
            Component *c = ds->getComponent(compit->first);
            std::map<unsigned int, std::string> pilist = c->getPropertyList();
            for(std::map<unsigned int, std::string>::iterator piit = pilist.begin();
                    piit != pilist.end(); ++piit){
                Property* p = ds->getProperty(piit->first);
                for(unsigned int i = 0; i < compit->second; i++){
                    propranking[p->getRank()][p->getPropertyId()].push_back(piit->second);
                }
      }

    }

    std::map<unsigned int, PropertyValue> propertyvalues;

    for(std::map<unsigned int, std::map<unsigned int, std::list<std::string> > >::iterator rpiit = propranking.begin();
	rpiit != propranking.end(); ++rpiit){
      std::map<unsigned int, std::list<std::string> > pilist = rpiit->second;
      std::set<PropertyValue> localvalues;
      for(std::map<unsigned int, std::list<std::string> >::iterator piit = pilist.begin();
	  piit != pilist.end(); ++piit){
	PropertyValue propval;
	propval.setPropertyId(piit->first);
	std::list<double> listvals;
	std::list<std::string> lambdas = piit->second;
	for(std::list<std::string>::iterator itlamb = lambdas.begin();
	    itlamb != lambdas.end(); ++itlamb){
	  temp = scheme_eval_string((std::string("(") + (*itlamb) + " design)").c_str(), env);
	  if(!SCHEME_NUMBERP(temp)){
	    Logger::getLogger()->warning("MzScheme: Return not a number");
	  }else{
	    listvals.push_back(scheme_real_to_double(temp));
	  }
	}
	Property *p = ds->getProperty(piit->first);
	formater.str("");
	formater << "(" <<  p->getTpclDisplayFunction() << " design '(";
	for(std::list<double>::iterator itvals = listvals.begin();
	    itvals != listvals.end(); ++itvals){
	  formater << *itvals << " ";
	}
	formater << "))";
	temp = scheme_eval_string(formater.str().c_str(), env);
#ifdef HAVE_MZSCHEME20X
	if(!SCHEME_PAIRP(temp) || !SCHEME_NUMBERP(SCHEME_CAR(temp)) || !SCHEME_STRINGP(SCHEME_CDR(temp))){
#else
	if(!SCHEME_PAIRP(temp) || !SCHEME_NUMBERP(SCHEME_CAR(temp)) || !SCHEME_CHAR_STRINGP(SCHEME_CDR(temp))){
#endif
	  Logger::getLogger()->warning("MzScheme: Return not a pair, or the wrong time in the pair");
	}else{
	  propval.setValue(scheme_real_to_double(SCHEME_CAR(temp)));
#ifdef HAVE_MZSCHEME20X
	  propval.setDisplayString(std::string(SCHEME_STR_VAL(SCHEME_CDR(temp)))); 
#else
	  propval.setDisplayString(std::string((char*)SCHEME_CHAR_STR_VAL(SCHEME_CDR(temp)))); 
#endif
	  localvalues.insert(propval);
	}
      }
      for(std::set<PropertyValue>::iterator pvit = localvalues.begin();
	  pvit != localvalues.end(); ++pvit){
	PropertyValue pv = *pvit;
	formater.str("");
	formater << "(property-designType-set! design " 
		 << pv.getPropertyId() << " " << pv.getValue()
		 << ")";
	temp = scheme_eval_string(formater.str().c_str(), env);
	propertyvalues[pv.getPropertyId()] = pv;
      }
    }

    d->setPropertyValues(propertyvalues);

    // now check if the design is valid
    
    bool valid = true;
    std::string feedback = "";
        Logger::getLogger()->debug("About to process requirement functions");

        for(std::map<unsigned int, unsigned int>::iterator compit = complist.begin();
                compit != complist.end();
                ++compit){
            unsigned int curval = compit->first;
      
      //for each component in the design
      temp = scheme_eval_string((std::string("(") + ds->getComponent(curval)->getTpclRequirementsFunction() + " design)").c_str(), env);
#ifdef HAVE_MZSCHEME20X
      if(!SCHEME_PAIRP(temp) || !SCHEME_STRINGP(SCHEME_CDR(temp))){
#else
      if(!SCHEME_PAIRP(temp) || !SCHEME_CHAR_STRINGP(SCHEME_CDR(temp))){
#endif
	Logger::getLogger()->warning("MzScheme: (a) Return not a pair, or the wrong time in the pair");
      }else{
	valid &= SCHEME_TRUEP(SCHEME_CAR(temp));
#ifdef HAVE_MZSCHEME20X
	std::string strtemp = SCHEME_STR_VAL(SCHEME_CDR(temp));
#else
	std::string strtemp = (char*)SCHEME_CHAR_STR_VAL(SCHEME_CDR(temp));
#endif
	if(strtemp.length() > 0)
	  feedback += strtemp + " ";
      }
    }

        for(std::map<unsigned int, std::map<unsigned int, std::list<std::string> > >::iterator rpiit = propranking.begin();
                rpiit != propranking.end(); ++rpiit){
            std::map<unsigned int, std::list<std::string> > pilist = rpiit->second;
            for(std::map<unsigned int, std::list<std::string> >::iterator piit = pilist.begin();
                    piit != pilist.end(); ++piit){
                temp = scheme_eval_string((std::string("(") + ds->getProperty(piit->first)->getTpclRequirementsFunction() + " design)").c_str(), env);
#ifdef HAVE_MZSCHEME20X
                if(!SCHEME_PAIRP(temp) || !SCHEME_STRINGP(SCHEME_CDR(temp))){
#else
                if(!SCHEME_PAIRP(temp) || !SCHEME_CHAR_STRINGP(SCHEME_CDR(temp))){
#endif
                    Logger::getLogger()->warning("MzScheme: (a) Return not a pair, or the wrong time in the pair");
                }else{
                    valid &= SCHEME_TRUEP(SCHEME_CAR(temp));
#ifdef HAVE_MZSCHEME20X
                    std::string strtemp = SCHEME_STR_VAL(SCHEME_CDR(temp));
#else
                    std::string strtemp = (char*)SCHEME_CHAR_STR_VAL(SCHEME_CDR(temp));
#endif
                    if(strtemp.length() > 0)
                        feedback += strtemp + " ";
                }
            }
        }

        propranking.clear();

    d->setValid(valid, feedback);
    

    Logger::getLogger()->debug("Eval'ed design");
    if(!valid){
        Logger::getLogger()->debug("Design %s is not valid, reason: %s", d->getName().c_str(), feedback.c_str());
    }
  }
  
}

TpMzScheme::TpMzScheme(){
    //scheme_set_stack_base(NULL, 1); /* required for OS X, only. WILL NOT WORK HERE */
    bool loaded = false;
  env = scheme_basic_env();
  if (scheme_setjmp(scheme_error_buf)) {
    Logger::getLogger()->warning("MzScheme Error");
  } else {
      scheme_eval_string("(load \"../modules/tpcl/mzscheme/designstruct.scm\")",env);
        loaded = true;
  }
    if(loaded == false){
        if (scheme_setjmp(scheme_error_buf)) {
            Logger::getLogger()->warning("MzScheme Error");
        } else {
            scheme_eval_string("(load \"" DATADIR "/designstruct.scm\")", env);
            loaded = true;
        }
    }
    if(loaded == false){
        Logger::getLogger()->warning("MzScheme Error");
        //throw exception?
    }
}
