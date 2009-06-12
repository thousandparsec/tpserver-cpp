/*  Logging for tpserver-cpp
 *
 *  Copyright (C) 2003-2005, 2007  Lee Begg and the Thousand Parsec Project
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

#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "filelogger.h"

// TODO: better exception support

FileLogger::FileLogger() {
  filename = "/var/log/tpserver-cpp.log";
  stream = new std::ofstream(filename.c_str(), std::ios_base::out);
  if (!stream) {
    throw std::exception();
  }
}

FileLogger::FileLogger(std::string logfile) {
  filename = logfile;
  stream = new std::ofstream(filename.c_str(), std::ios_base::out);
  if (!stream) {
    throw std::exception();
  }
}

FileLogger::~FileLogger() {
  stream->close();
  delete stream;
}


void FileLogger::doLogging( int level, const char* msg) const {
  char   timeStr[40];
  time_t currTime = time( NULL);
  std::ostringstream levelStr;
  static const std::string levelStrings[] = { 
    " < Debug > ",
    " < Info  > ",
    " <Warning> ",
    " < Error > "
  };

  strftime( timeStr, 40, "%F %H:%M:%S", localtime( &currTime));

  if (level <= 3) {
    levelStr << levelStrings[level];
  } else {
    levelStr << " <  " << level << "  > ";
  }

  *stream << timeStr << levelStr.str() << msg << std::endl;
}
