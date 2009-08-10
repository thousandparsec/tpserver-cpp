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

/**
 * Frame class for representing TP protocol frames
 */
class Frame {
  public:
    /**
     * Standard constructor
     *
     * Creates version 3 frames
     */
    Frame();

    /**
     * Constructor setting a given version
     */
    explicit Frame(ProtocolVersion v);

    /**
     * Destructor
     *
     * Frees data if needed
     */
    ~Frame();


    /**
     * CREATE header??
     */
    // TODO: This is something to be refactored!
    int setHeader(const std::string& new_header);

    // TODO: once data is internall held as a string make it return a const std::string&
    std::string getPacket() const;

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

    // Data
    bool setData( const std::string& new_data );

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
    bool packInt64(int64_t val);
    bool packInt8(char val);
    bool packIdModList(const IdModList& modlist, uint32_t count = 0, uint32_t from_position = 0 );
    bool packIdSet(const IdSet& idset);
    bool packIdMap(const IdMap& idmap);
    bool packIdStringMap(const IdStringMap& idmap);

    bool isEnoughRemaining(uint32_t size) const;
    /// Advances the frame position by amount bytes
    void advance(uint32_t amount);

    int unpackInt();
    std::string unpackStdString();
    int64_t unpackInt64();
    char unpackInt8();
    IdMap unpackMap();
    IdSet unpackIdSet();

  private:
    /// Version of protocol that this frame is encoded with
    ProtocolVersion version;
    /// Frame type
    FrameType type;
    /// Type version
    uint32_t typeversion;
    /// Which packet sequence does this refer to?
    uint32_t sequence;
    /// Frame length
    uint32_t length;
    /// Actual data of the frame
    char *data;
    /// Whether to pad strings with \0 values
    bool padstrings;
    /// Current unpack position
    uint32_t unpackptr;
 
    /// Blocked copy constructor
    Frame(const Frame &rhs) {};
    /// Blocked assignemnt operator
    Frame operator=(const Frame &rhs) { return Frame(); };

};

#endif
