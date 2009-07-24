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
#include <sstream>
#include <boost/bind.hpp>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "consolelogger.h"
#include "settings.h"

ConsoleLogger::ConsoleLogger() {
  Settings::getSettings()->setCallback("log_colour", boost::bind( &ConsoleLogger::reconfigure, this, _1, _2 ));
  reconfigure("", "");
}

ConsoleLogger::~ConsoleLogger() {
  Settings::getSettings()->removeCallback("log_colour");
}

void ConsoleLogger::reconfigure(const std::string& key, const std::string& value) {
  colour = (Settings::getSettings()->get("log_colour") == "yes");
}

void ConsoleLogger::doLogging( int level, const char* msg) const {
  char        timeStr[30];
  time_t      currTime = time( NULL);
  std::ostringstream levelStr;
  
  static const std::string colourStrings[] = { 
    "",
    "\e[32;1m",
    "\e[33;1m",
    "\e[31;1m"
  };

  static const std::string levelStrings[] = { 
    " < Debug > ",
    " < Info  > ",
    " <Warning> ",
    " < Error > "
  };

  strftime( timeStr, 30, "%F %H:%M:%S", localtime( &currTime));

  if (level <= 3) {
    if (colour) levelStr << colourStrings[level];
    levelStr << levelStrings[level];
    if (colour) levelStr << "\e[0m";
  }
  else
    levelStr << " <  " << level << "  > ";

  std::cout << "\r" << timeStr << levelStr.str() << msg << std::endl;
}
