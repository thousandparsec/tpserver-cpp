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

class LogSink;

class Logger {

 public:
	static Logger *getLogger();

	void debug(const char *msg, ...);
	void info(const char *msg, ...);
	void warning(const char *msg, ...);
	void error(const char *msg, ...);

	int addLog(LogSink* newlog);
	void removeLog(int extid);

	void flush();

	void reconfigure(const std::string & item, const std::string & value);


 private:
    Logger();
	virtual ~Logger();
    Logger(Logger & rhs);
	Logger operator=(Logger & rhs);

	void doLogging( int level, const char *msg);

    std::map<std::string, LogSink*>  logSinkMap;

        int extcount;
        
	int loglevel;

	static Logger *myInstance;
};

#define debugLog   Logger::getLogger()->debug
#define infoLog    Logger::getLogger()->info
#define warningLog Logger::getLogger()->warning
#define errorLog   Logger::getLogger()->error

#endif
