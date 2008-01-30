/*  ObjectView class
 *
 *  Copyright (C) 2008 Lee Begg and the Thousand Parsec Project
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

#include "objectview.h"

ObjectView::ObjectView(): objid(0), completelyvisible(false), dead(false), seename(false),
                              visiblename(), seedesc(false), visibledesc(){
  timestamp = time(NULL);
}

ObjectView::~ObjectView(){

}

void ObjectView::packFrame(Frame* frame) const{
  if(dead){
    frame->createFailFrame(fec_NonExistant, "No such object");
  }else{
     //TODO write this function
    
  }
}

uint32_t ObjectView::getObjectId() const{
  return objid;
}

bool ObjectView::isCompletelyVisible() const{
  return completelyvisible;
}

bool ObjectView::isDead() const{
  return dead;
}

bool ObjectView::canSeeName() const{
  return seename;
}

std::string ObjectView::getVisibleName() const{
  return visiblename;
}

bool ObjectView::canSeeDescription() const{
  return seedesc;
}

std::string ObjectView::getVisibleDescription() const{
    return visibledesc;
}

uint64_t ObjectView::getModTime() const{
    return timestamp;
}

void ObjectView::setObjectId(uint32_t id){
  objid = id;
  touchModTime();
}

void ObjectView::setCompletelyVisible(bool ncv){
  if(ncv != completelyvisible)
    touchModTime();
  completelyvisible = ncv;
}

void ObjectView::setDead(bool nid){
  if(nid != dead){
    touchModTime();
    dead = nid;
  }
}

void ObjectView::setCanSeeName(bool csn){
  if(csn != seename)
    touchModTime();
  seename = csn;
}

void ObjectView::setVisibleName(const std::string& nvn){
  visiblename = nvn;
  touchModTime();
}

void ObjectView::setCanSeeDescription(bool csd){
  if(csd != seedesc)
    touchModTime();
  seedesc = csd;
}

void ObjectView::setVisibleDescription(const std::string& nvd){
  visibledesc = nvd;
  touchModTime();
}


void ObjectView::setModTime(uint64_t nmt){
    timestamp = nmt;
}

void ObjectView::touchModTime(){
  timestamp = time(NULL);
}
