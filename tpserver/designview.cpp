/*  DesignView class
 *
 *  Copyright (C) 2005,2006, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <time.h>

#include "frame.h"
#include "game.h"
#include "designstore.h"
#include "design.h"

#include "designview.h"

DesignView::DesignView() : ProtocolView( ft03_Design ), 
    seenum(false), exist(0), seeowner(false), owner(0) 
{
}

DesignView::DesignView( uint32_t desid, bool visibility ) : ProtocolView( ft03_Design ),
    seenum(false), exist(0), seeowner(false), owner(0) 
{
  setId( desid );
  completely_visible = visibility;
}

DesignView::~DesignView(){

}

void DesignView::pack(Frame* frame) const{
  
  Design* design = Game::getGame()->getDesignStore()->getDesign(id);
  
  frame->setType(ft03_Design);
  frame->packInt(id);
  if(completely_visible && getModTime() < design->getModTime()){
    frame->packInt64(design->getModTime());
  }else{
    frame->packInt64(getModTime());
  }
  frame->packInt(1);
  frame->packInt(design->getCategoryId());
  if(completely_visible || name_visible){
    frame->packString(design->getName());
  }else{
    frame->packString(name);
  }
  if(completely_visible || desc_visible){
    frame->packString(design->getDescription());
  }else{
    frame->packString(desc);
  }
  if(completely_visible || seenum){
    frame->packInt(design->isValid() ? design->getInUse() : UINT32_NEG_ONE);
  }else{
    frame->packInt(exist);
  }
  if(completely_visible || seeowner){
    frame->packInt(design->getOwner());
  }else{
    frame->packInt(owner);
  }
  IdMap complist;
  if(completely_visible){
    complist = design->getComponents();
  }else{
    complist = components;
  }
  frame->packIdMap(complist);
  if(completely_visible){
    frame->packString(design->getFeedback());
  }else{
    frame->packString("");
  }
  PropertyValue::Map proplist;
  if(completely_visible){
    proplist = design->getPropertyValues();
  }else{
    proplist = properties;
  }
  frame->packInt(proplist.size());
  for(PropertyValue::Map::const_iterator itcurr = proplist.begin();
      itcurr != proplist.end(); ++itcurr){
    itcurr->second.packFrame(frame);
  }
}

uint32_t DesignView::getDesignId() const{
  return id;
}

uint32_t DesignView::getVisibleOwner() const{
  return owner;
}

bool DesignView::canSeeOwner() const{
  return seeowner;
}

IdMap DesignView::getVisibleComponents() const{
  return components;
}

uint32_t DesignView::getVisibleNumExist() const{
  return exist;
}

bool DesignView::canSeeNumExist() const{
  return seenum;
}

PropertyValue::Map DesignView::getVisiblePropertyValues() const{
    return properties;
}

void DesignView::setDesignId(uint32_t id){
  setId( id );
  touchModTime();
}

void DesignView::setVisibleOwner(uint32_t o){
  owner = o;
  touchModTime();
}

void DesignView::setCanSeeOwner(bool cso){
  seeowner = cso;
  touchModTime();
}

void DesignView::setVisibleComponents(IdMap cl){
  components = cl;
  touchModTime();
}

void DesignView::setVisibleNumExist(uint32_t nne){
    exist = nne;
    touchModTime();
}

void DesignView::setCanSeeNumExist(bool csn){
  seenum = csn;
  touchModTime();
}

void DesignView::setVisiblePropertyValues(PropertyValue::Map pvl){
  properties = pvl;
  touchModTime();
}

