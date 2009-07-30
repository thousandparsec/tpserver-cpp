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

#include <stdint.h>
#include <tpserver/protocolview.h>
#include <tpserver/common.h>

class Frame;

class ComponentView : public ProtocolView {
 public:
  typedef boost::shared_ptr< ComponentView > Ptr;
  ComponentView();
  ComponentView( uint32_t new_id, bool visibility );
  virtual ~ComponentView();

  void pack(Frame* frame) const;

  uint32_t getComponentId() const;
  
  IdSet getVisibleCategories() const;
  IdSet getVisiblePropertyFuncs() const;
  bool canSeeRequirementsFunc() const;
  
  void setComponentId(uint32_t id);
  
  void setVisibleCategories(const IdSet& nvc);
  void addVisibleCategory(uint32_t catid);
  void setCanSeeRequirementsFunc(bool csr);
  void setVisiblePropertyFuncs(const IdSet& nvp);
  void addVisiblePropertyFunc(uint32_t propid);
  
 protected:
  bool requirements_visible;
  IdSet cats_visible;
  IdSet properties_visible;
  

};


#endif
