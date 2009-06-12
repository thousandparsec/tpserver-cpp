#ifndef CONSOLELOGGER_H
#define CONSOLELOGGER_H
/*  Logger class for logging to the console
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

#include <tpserver/logsink.h>

/**
 * Log sink that logs to console
 *
 * @see LogSink
 */
class ConsoleLogger : public LogSink {
  public:
    /**
     * Default constructor
     *
     * Registers the log_colour callback to settings.
     */
    ConsoleLogger();

    /**
     * Destructor
     *
     * Unregisters settings callback.
     */
    virtual ~ConsoleLogger();

    /**
     * Logging override
     *
     * Logs to console.
     */
    virtual void doLogging( int level, const char* msg) const;

  private:
    /// Callback for reconfiguring color
    void reconfigure(const std::string& key, const std::string& value);

    /// Is console output colored?
    bool colour;
};


#endif
