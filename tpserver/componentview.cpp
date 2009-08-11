/*  ComponentView class
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

#include <time.h>

#include "frame.h"
#include "game.h"
#include "component.h"
#include "designstore.h"

#include "componentview.h"

ComponentView::ComponentView(): ProtocolView(ft03_Component), requirements_visible(false)
{
}

ComponentView::ComponentView( uint32_t new_id, bool visibility ): ProtocolView(ft03_Component), requirements_visible(false)
{
  setId( new_id );
  setCompletelyVisible( visibility );
}

ComponentView::~ComponentView(){

}

void ComponentView::pack(OutputFrame* frame) const{
  
  Component::Ptr comp = Game::getGame()->getDesignStore()->getComponent(id);
  
  frame->setType(ft03_Component);
  frame->packInt(id);
  if(completely_visible && getModTime() < comp->getModTime()){
    frame->packInt64(comp->getModTime());
  }else{
    frame->packInt64(getModTime());
  }
  IdSet catids;
  if(completely_visible){
    catids = comp->getCategoryIds();
  }else{
    catids = cats_visible;
  }
  //Maybe: filter catids list for non-visible categories?
  frame->packIdSet(catids);
  if(completely_visible || name_visible){
    frame->packString(comp->getName());
  }else{
    frame->packString(name);
  }
  if(completely_visible || desc_visible){
    frame->packString(comp->getDescription());
  }else{
    frame->packString(desc);
  }
  if(completely_visible || requirements_visible){
    frame->packString(comp->getTpclRequirementsFunction());
  }else{
    frame->packString("(lamda (design) (cons #f \"Unknown Component. \")))");
  }
  IdStringMap propertylist;
  if(completely_visible){
    propertylist = comp->getPropertyList();
  }else{
    // TODO: WTF? isn't this the same???
    IdStringMap compproplist = comp->getPropertyList();
    for(IdSet::const_iterator itcurr = properties_visible.begin();
        itcurr != properties_visible.end(); ++itcurr){
      propertylist[*itcurr] = compproplist[*itcurr];
    }
  }
  frame->packIdStringMap(propertylist);
}

uint32_t ComponentView::getComponentId() const{
  return id;
}

IdSet ComponentView::getVisibleCategories() const{
  //maybe: filter out non-visible cats
  return cats_visible;
}

bool ComponentView::canSeeRequirementsFunc() const{
  return requirements_visible;
}


IdSet ComponentView::getVisiblePropertyFuncs() const{
  return properties_visible;
}

void ComponentView::setComponentId(uint32_t id){
  setId(id);
  touchModTime();
}

void ComponentView::setVisibleCategories(const IdSet& nvc){
  cats_visible = nvc;
  touchModTime();
}

void ComponentView::addVisibleCategory(uint32_t catid){
  cats_visible.insert(catid);
  touchModTime();
}


void ComponentView::setCanSeeRequirementsFunc(bool csr){
  if(csr != requirements_visible)
    touchModTime();
  requirements_visible = csr;
}

void ComponentView::setVisiblePropertyFuncs(const IdSet& nvp){
  properties_visible = nvp;
  touchModTime();
}

void ComponentView::addVisiblePropertyFunc(uint32_t propid){
  properties_visible.insert(propid);
  touchModTime();
}

