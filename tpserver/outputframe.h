#ifndef OUTPUTFRAME_H
#define OUTPUTFRAME_H
/*  TP protocol OutputFrame class
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

#include <tpserver/frame.h>
#include <boost/shared_ptr.hpp>

class OutputFrame : public Frame {
  public:
    typedef boost::shared_ptr<OutputFrame> Ptr;
    /**
     * Constructor setting a given version
     */
    explicit OutputFrame(ProtocolVersion v = fv0_3, bool padding = false);
    
    std::string getPacket() const;
    
    bool setType(FrameType nt);
    void setTypeVersion(uint32_t tv);
    void setSequence(int s);
    void setPadding(bool new_padding);

    void packString(const std::string &str);
    void packInt(int val);
    void packInt64(int64_t val);
    void packInt8(char val);
    void packIdModList(const IdModList& modlist, uint32_t count = 0, uint32_t from_position = 0 );
    void packIdSet(const IdSet& idset);
    void packIdMap(const IdMap& idmap);
    void packIdStringMap(const IdStringMap& idmap);

};

#endif
