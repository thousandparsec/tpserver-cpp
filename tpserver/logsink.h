#ifndef LOGSINK_H
#define LOGSINK_H
/*  Virtual class for logging destinations
 *
 *  Copyright (C) 2004-2005, 207  Lee Begg and the Thousand Parsec Project
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

#include <boost/shared_ptr.hpp>

/**
 * Log sink abstract interface class
 */
class LogSink {
  public:
    typedef boost::shared_ptr< LogSink > Ptr;
    /**
     * Empty virtual destructor to prevent compiler warnings
     */
    virtual ~LogSink(){};

    /**
     * Abstract logging function
     *
     * @param level logging level
     * @param msg logging message
     */
    virtual void doLogging( int level, const char* msg) const = 0;
};

#endif
