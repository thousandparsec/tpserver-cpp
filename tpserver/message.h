#ifndef MESSAGE_H
#define MESSAGE_H
/*  Messages board Message class
 *
 *  Copyright (C) 2004-2005  Lee Begg and the Thousand Parsec Project
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

#include <tpserver/refsys.h>
#include <tpserver/protocolobject.h>

class Frame;

/**
 * Class representing a readable message posted on message boards in TP
 *
 * A message consists of a subject, body, turn number and references.
 */
class Message : public ProtocolObject {
  public:
    /// Type definition for a single reference
    typedef std::pair<RefSysType, uint32_t> Ref;
    
    /// Type definition for a set of references
    typedef std::set<Ref> References;

    /// Type defininition for shared pointer
    typedef boost::shared_ptr<Message> Ptr;

  public:
    /**
     * Default constructor
     *
     * Sets the turn number to current game turn.
     */
    Message();

    /// Returns the stored turn value
    int getTurn();

    /// Sets the stored turn value
    void setTurn(uint32_t nt);

    /// Sets the subject
    void setSubject(const std::string &nsub);

    /// Returns the subject
    std::string getSubject();

    /// Sets the message body
    void setBody(const std::string &nbody);

    /// Returns the message body
    std::string getBody();

    /**
     * Adds a new reference to the stored references.
     *
     * @param type Reference type (RefSysType)
     * @param value Reference value
     */
    void addReference(RefSysType type, uint32_t value);

    /**
     * Returns the reference set
     */
    References getReferences() const;

    /**
     * Packs the Message into a frame
     */
    void pack(Frame * frame) const;

    void setId( uint32_t new_id ) { id = new_id; }
  private:

    /// Turn number when the message was posted
    int turn_number;

    /// Message references
    References references;
};

#endif
