#ifndef COMPONENTVIEW_H
#define COMPONENTVIEW_H
/*  Design ComponentView class
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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

#include <string>
#include <set>
#include <stdint.h>

class Frame;

class ComponentView{
 public:
  ComponentView();
  virtual ~ComponentView();

  void packFrame(Frame* frame) const;

  uint32_t getComponentId() const;
  uint64_t getModTime() const;
  bool isCompletelyVisible() const;
  
  std::set<uint32_t> getVisibleCategories() const;
  bool canSeeName() const;
  std::string getVisibleName() const;
  bool canSeeDescription() const;
  std::string getVisibleDescription() const;
  bool canSeeRequirementsFunc() const;
  std::set<uint32_t> getVisiblePropertyFuncs() const;
  

  void setComponentId(uint32_t id);
  void setModTime(uint64_t nmt);
  void setCompletelyVisible(bool ncv);
  
  void setVisibleCategories(const std::set<uint32_t>& nvc);
  void addVisibleCategory(uint32_t catid);
  void setCanSeeName(bool csn);
  void setVisibleName(const std::string& nvn);
  void setCanSeeDescription(bool csd);
  void setVisibleDescription(const std::string& nvd);
  void setCanSeeRequirementsFunc(bool csr);
  void setVisiblePropertyFuncs(const std::set<uint32_t>& nvp);
  void addVisiblePropertyFunc(uint32_t propid);
  
  
 protected:
   void touchModTime();
   
  uint32_t compid;
  uint64_t timestamp;
  bool completelyvisible;
  
  std::set<uint32_t> visiblecats;
  
  bool seename;
  std::string visiblename;
  
  bool seedesc;
  std::string visibledesc;
  
  bool seerequirements;
  
  std::set<uint32_t> visibleproperties;
  

};


#endif
