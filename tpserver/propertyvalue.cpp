/*  PropertyValue class
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

#include "frame.h"

#include "propertyvalue.h"


PropertyValue::PropertyValue() : propid( 0 ), value( 0.0 ) 
{}                                                          

PropertyValue::PropertyValue( uint32_t pid, double pval ) : propid( pid ), value( pval ) {
}

PropertyValue::PropertyValue(const PropertyValue& rhs){
  propid = rhs.propid;
  value = rhs.value;
  display = rhs.display;
}

PropertyValue::~PropertyValue(){

}

PropertyValue PropertyValue::operator=(const PropertyValue& rhs){
  propid = rhs.propid;
  value = rhs.value;
  display = rhs.display;
  return *this;
}

bool PropertyValue::operator==(const PropertyValue& rhs) const{
  return (propid == rhs.propid);
}

bool PropertyValue::operator<(const PropertyValue& rhs) const{
  return (propid < rhs.propid);
}

void PropertyValue::packFrame(Frame* frame) const{
  frame->packInt(propid);
  frame->packString(display);
}

uint32_t PropertyValue::getPropertyId() const{
  return propid;
}

double PropertyValue::getValue() const{
  return value;
}

std::string PropertyValue::getDisplayString() const{
  return display;
}

void PropertyValue::setPropertyId(uint32_t id){
  propid = id;
}

void PropertyValue::setValue(double v){
  value = v;
}

void PropertyValue::setDisplayString(const std::string& d){
  display = d;
}
