#ifndef CONNECTION_H
#define CONNECTION_H
/*  Connection base class
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include <boost/enable_shared_from_this.hpp>

/**
 * Abstract connection class
 */
class Connection : public boost::enable_shared_from_this< Connection > {
  public:
    /// Shared pointer to connection typedef
    typedef boost::shared_ptr< Connection > Ptr;

    /**
     * Connection status
     */
    enum Status {
      DISCONNECTED = 0, //!< Connection not established
      PRECONNECTED,     //!< Connection established but not ready
      CONNECTED,        //!< Connection established
      READY             //!< Connection ready
    };

    /**
     * Default constructor
     *
     * Zeroes all fields
     */
    Connection();
    
    /**
     * Explicit FD constructor
     *
     * Sets status to preconnected, and fd to the passed value
     */
    explicit Connection(int fd);


    /**
     * Destructor
     *
     * Does nothing but ensures virtuality
     */
    virtual ~Connection();

    /**
     * Abstract processing
     */
    virtual void process() = 0;

    /**
     * Virtual process write
     */
    virtual void processWrite();

    /**
     * Returns status
     */
    Status getStatus();

    /**
     * Returns file descriptor of connection
     */
    // TODO: Remove!
    int getFD();

  protected:
    /// Connection socket file descriptor
    int sockfd;
    /// Connection status
    Status status;
};

#endif
