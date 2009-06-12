#ifndef ADMINLOGGER_H
#define ADMINLOGGER_H
/*  Logger class for logging to the admin client
 *
 *  Copyright (C) 2008 Aaron Mavrinac and the Thousand Parsec Project
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

#include <tpserver/logsink.h>

class AdminConnection;

/**
 * Log sink that logs via admin connection
 *
 * @see LogSink
 * @see AdminConnection
 */
class AdminLogger : public LogSink {
  public:
    /**
     * Default constructor
     */
    AdminLogger();

    /**
     * Destructor
     */
    virtual ~AdminLogger();

    /**
     * Sets the connection
     */
    void setConnection(AdminConnection * newcon);

    /**
     * Retrieves the connection
     */
    AdminConnection *getConnection() const;

    /**
     * Logging override
     *
     * Logs to admin connection.
     */
    virtual void doLogging(int level, const char* msg) const;

  private:
    /// Stored admin connection
    AdminConnection* connection;

};


#endif
