#ifndef FILELOGGER_H
#define FILELOGGER_H
/*  Logger class for logging to a file
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

#include <iostream>
#include <fstream>

#include <tpserver/logsink.h>

/**
 * Log sink that logs to file
 *
 * @see LogSink
 */
class FileLogger : public LogSink {
  public:
    /**
     * Default constructor
     *
     * Opens logger to default log-file
     */
    FileLogger();

    /**
     * Explicit constructor
     *
     * Opens file passed for logging
     */
    explicit FileLogger(std::string logfile);

    /**
     * Destructor
     *
     * Performs cleanup and flush.
     */
    virtual ~FileLogger();

    /**
     * Logging override
     *
     * Logs to opened file.
     */
    virtual void doLogging(int level, const char* msg) const;

  protected:
    /// Log file name
    std::string     filename;
    /// Log file stream
    std::ofstream*  stream;
};


#endif
