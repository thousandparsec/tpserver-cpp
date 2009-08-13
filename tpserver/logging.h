#ifndef LOGGER_H
#define LOGGER_H
/*  Logger class for server internal logging
 *
 *  Copyright (C) 2004-2005, 2006  Lee Begg and the Thousand Parsec Project
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

#include <string>
#include <map>
#include <tpserver/logsink.h>

/**
 * Logger singleton for internal logging
 */
class Logger {

  public:
    /**
     * Static singleton instance accessor
     */
    static Logger *getLogger();

    /**
     * Write to debug log
     */
    void debug(const char *msg, ...);

    /**
     * Write to info log
     */
    void info(const char *msg, ...);

    /**
     * Write to warning log
     */
    void warning(const char *msg, ...);

    /**
     * Write to error log
     */
    void error(const char *msg, ...);

    /**
     * Add a log sink
     */
    int addLog(LogSink::Ptr newlog);

    /**
     * Remove log sink
     */
    void removeLog(int extid);

    /**
     * Flush the log sinks
     */
    void flush();

    /**
     * Callback for Settings configuration
     */
    void reconfigure(const std::string & item, const std::string & value);

  private:
    /// Private constructor, class can only be instantiated via getLogger
    Logger();

    /// Private destructor (how will the class be freed??)
    virtual ~Logger();

    /// Blocked copy constructor
    Logger(Logger & rhs);

    /// Blocked assignment operator
    Logger operator=(Logger & rhs);

    /// Perform logging per operation level
    void doLogging( int level, const char *msg);

  private:
    /// Sink map typedef
    typedef std::map<std::string, LogSink::Ptr> SinkMap; 

    /// Sink map
    SinkMap sink_map;

    /// Count of external sinks 
    int sink_count;

    /// Current log level
    int log_level;

    /// Singleton static instance
    static Logger *instance;
};

#define debugLog   Logger::getLogger()->debug
#define infoLog    Logger::getLogger()->info
#define warningLog Logger::getLogger()->warning
#define errorLog   Logger::getLogger()->error

#define DEBUG   Logger::getLogger()->debug
#define INFO    Logger::getLogger()->info
#define WARNING Logger::getLogger()->warning
#define ERROR   Logger::getLogger()->error

#endif
