#ifndef FILELOGGER_H
#define FILELOGGER_H
/*  Logger class for logging to a file
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

#include <iostream>
#include <fstream>

#include <tpserver/logsink.h>

class FileLogger : public LogSink {
 public:
    FileLogger();
    FileLogger( std::string  logfile);
    virtual ~FileLogger();

    virtual void doLogging( int level, char* msg) const;
    virtual void reconfigure();

 protected:
    std::string    logFileSpec;
    std::ofstream*      logStream;

 private:

};


#endif
