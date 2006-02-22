#ifndef TPGUILE_H
#define TPGUILE_H
/*  TP Guile interpreter class
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include <map>
#include <list>
#include <string>

#include <tpserver/tpscheme.h>

#include <tpserver/property.h>
#include <tpserver/propertyvalue.h>

class Design;
typedef struct Scheme_Env Scheme_Env;

class TpGuile : public TpScheme{
 public:
  static TpGuile* getImplemention();

        virtual ~TpGuile();
  virtual void evalDesign(Design* d);

  // Comment the following line out for building guile_test
 private:
  void definePropertyDesignTypeSet();
  void defineDesignType( Design * d);
  void definePropertyAccessors();
  void setDesignPropertyValue( const PropertyValue & pv);
  void setDesignComponentCount( const unsigned int count);
  std::map<unsigned int, std::map<unsigned int, std::list<std::string> > > *
      createPropertyRankingMap( std::map<unsigned int, unsigned int> & complist);
  double evalCompProperty( std::string lambdaStr);
  PropertyValue getPropertyValue( Property * p,
                                  std::list<std::string> & compPropStrList);
  std::map<unsigned int, PropertyValue> * calculateDesignPropertyValues( Design* d);
  bool evalRequirementFtn( std::string function, std::string & why);
  bool canPropGoInDesign( unsigned int propertyID, std::string & why);
  bool canCompGoInDesign( unsigned int componentID, std::string & why);
  void validateDesign( Design* d,
                       std::map<unsigned int, PropertyValue> & propertyvalues);


 private:
  TpGuile();

  static TpGuile* instance;
};

#endif
