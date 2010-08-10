#ifndef PROTOCOLVIEW_H
#define PROTOCOLVIEW_H
/*  ProtocolView base class
 *
 *  Copyright (C) 2009 Kornel Kisielewicz and the Thousand Parsec Project
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

#include <tpserver/protocolobject.h>

class ProtocolView : public ProtocolObject {
  public:
    typedef boost::shared_ptr< ProtocolView > Ptr;
    ProtocolView( FrameType frame_type ) 
      : ProtocolObject( frame_type, 0, "", "" ), completely_visible( false ), name_visible( false ), desc_visible( false ), gone(false) {}
    virtual ~ProtocolView() {}
    bool isCompletelyVisible() const { return completely_visible; }
    bool canSeeName() const { return name_visible; }
    std::string getVisibleName() const { return name; }
    bool canSeeDescription() const { return desc_visible; }
    std::string getVisibleDescription() const { return desc; }
    bool isGone() const { return gone; }

    void setCompletelyVisible(bool visibility) { completely_visible = visibility; touchModTime(); }
    void setCanSeeName(bool visibility) { name_visible = visibility; touchModTime(); }
    void setVisibleName(const std::string& new_name) { setName( new_name ); touchModTime(); }
    void setCanSeeDescription(bool visibility) { desc_visible = visibility; touchModTime(); }
    void setVisibleDescription(const std::string& new_desc) { setDescription( new_desc ); touchModTime(); }
    void setGone(bool isgone) { gone = isgone; touchModTime(); }
  protected:
    bool completely_visible;
    bool name_visible;
    bool desc_visible;
    bool gone;
};

#endif // PROTOCOLOBJECT_H
