#ifndef OBJECTVIEW_H
#define OBJECTVIEW_H
/*  ObjectView class
 *
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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

class ObjectView{
 public:
  ObjectView();
  virtual ~ObjectView();

  void packFrame(Frame* frame) const;

  uint32_t getObjectId() const;
  uint64_t getModTime() const;
  bool isCompletelyVisible() const;
  
  bool canSeeName() const;
  std::string getVisibleName() const;
  bool canSeeDescription() const;
  std::string getVisibleDescription() const;
  

  void setObjectId(uint32_t id);
  void setModTime(uint64_t nmt);
  void setCompletelyVisible(bool ncv);
  
  void setCanSeeName(bool csn);
  void setVisibleName(const std::string& nvn);
  void setCanSeeDescription(bool csd);
  void setVisibleDescription(const std::string& nvd);
  
  
  
 protected:
   void touchModTime();
   
  uint32_t objid;
  uint64_t timestamp;
  bool completelyvisible;
  
  bool seename;
  std::string visiblename;
  
  bool seedesc;
  std::string visibledesc;
  
  

};


#endif
