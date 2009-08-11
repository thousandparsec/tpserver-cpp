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


#include <tpserver/common.h>
#include <tpserver/frameexception.h>

/**
 * Frame class for representing TP protocol frames
 */
class Frame {
  public:
    /**
     * Destructor
     *
     * Frees data if needed
     */
    ~Frame();


    /**
     * Return the length of the header section
     */
    int getHeaderLength() const;

    /**
     * Return the length of the data section
     */
    int getDataLength() const;

    /**
     * Return the frame length
     *
     * Equals to length of data section + length of header section
     */
    int getLength() const;

    // Type
    FrameType getType() const;

    // frame type version
    uint32_t getTypeVersion() const;

    // Sequence
    int getSequence() const;

    // Version
    ProtocolVersion getVersion() const;

    //string padding
    bool isPaddingStrings() const;

  protected:
    /// Version of protocol that this frame is encoded with
    ProtocolVersion version;
    /// Frame type
    FrameType type;
    /// Type version
    uint32_t typeversion;
    /// Which packet sequence does this refer to?
    uint32_t sequence;
    /// Actual data of the frame
    std::string data;
    /// Whether to pad strings with \0 values
    bool padstrings;
 
    /**
     * Standard constructor
     */
    explicit Frame(ProtocolVersion v);

  private:
    /// Blocked default constructor
    Frame() {}
    /// Blocked copy constructor
    Frame(const Frame &rhs) {};
    /// Blocked assignemnt operator
    Frame operator=(const Frame &rhs) { return Frame(); };

};

class InputFrame : public Frame {
  public:
    /**
     * Standard constructor
     *
     * Creates version 3 frames
     */
    explicit InputFrame(ProtocolVersion v = fv0_3, bool padding = false);
    
    // Data
    void setData( const std::string& new_data );
   
    /**
     * CREATE header??
     */
    // TODO: This is something to be refactored!
    int setHeader(const std::string& new_header);
    
    bool isEnoughRemaining(uint32_t size) const;
    /// Advances the frame position by amount bytes
    void advance(uint32_t amount);

    int unpackInt();
    std::string unpackString();
    int64_t unpackInt64();
    char unpackInt8();
    IdMap unpackMap();
    IdSet unpackIdSet();


  private:
    /// Current unpack position
    uint32_t unpackptr;
};

class OutputFrame : public Frame {
  public:
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
