#ifndef FRAME_H
#define FRAME_H
/*  TP protocol Frame class
 *
 *  Copyright (C) 2004-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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
#include <string>
#include <list>

#include <tpserver/protocol.h>

class Frame {

  public:
    Frame();
    Frame(ProtocolVersion v);
    Frame(const Frame &rhs);
  
    ~Frame();
  
    Frame operator=(const Frame &rhs);
  
    int setHeader(char *newhead);
    char *getPacket() const;
    int getHeaderLength() const;		// The length of the header section
    int getDataLength() const;		// The length of the data section
    int getLength() const;			// The total length of the packet
    
    // Data
    char *getData() const;
    bool setData(char *newdata, int dlen);
  
    // Type
    FrameType getType() const;
    bool setType(FrameType nt);
    
    // frame type version
    uint32_t getTypeVersion() const;
    bool setTypeVersion(uint32_t tv);
    
    // Sequence
    int getSequence() const;
    bool setSequence(int s);
    
    // Version
    ProtocolVersion getVersion() const;
    
    //string padding
    bool isPaddingStrings() const;
    void enablePaddingStrings(bool on);
    
    bool packString(const std::string &str);
    bool packInt(int val);
    bool packInt64(long long val);
    bool packInt8(char val);
    bool packData(unsigned int len, char* bdata);
  
    bool isEnoughRemaining(uint32_t size) const;
    // uses these functions with care
    uint32_t getUnpackOffset() const;
    bool setUnpackOffset(uint32_t newoffset);
  
    int unpackInt();
    std::string unpackStdString();
    long long unpackInt64();
    char unpackInt8();
    void unpackData(unsigned int len, char* bdata);
  
    void createFailFrame(FrameErrorCode code, const std::string& reason);
    void createFailFrame(FrameErrorCode code, const std::string &reason, const std::list<std::pair<reftype_t, refvalue_t> > &refs);
  
  private:
    ProtocolVersion version;
    FrameType type;
    uint32_t typeversion;

    // Which packet sequence does this refer to?
    uint32_t sequence;
    
    // Frame length
    uint32_t length;

    // Actual data of the frame
    char *data;
    
    bool padstrings;

    uint32_t unpackptr;
};

#endif
