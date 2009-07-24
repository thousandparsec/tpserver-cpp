/*  Logging for tpserver-cpp
 *
 *  Copyright (C) 2003-2005, 2006, 2007  Lee Begg and the Thousand Parsec Project
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
#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include <cstdlib>
#include <boost/bind.hpp>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "settings.h"
#include "logging.h"
#include "filelogger.h"
#include "syslogger.h"
#include "consolelogger.h"

Logger *Logger::instance = NULL;

Logger *Logger::getLogger()
{
  if ( instance == NULL) {
    instance = new Logger();
  }

  return instance;
}

void Logger::debug(const char *msg, ...)
{
  if ( log_level <= 0) {
    char* fmsg = new char[100];

    va_list ap;
    va_start(ap, msg);
    int reallen = vsnprintf(fmsg, 100, msg, ap);
    if(reallen > 100){
      delete[] fmsg;
      fmsg = new char[reallen + 1];
      va_end(ap);
      va_start(ap, msg);
      vsnprintf(fmsg, reallen + 1, msg, ap);
    }
    va_end(ap);
    doLogging(0, fmsg);
    delete[] fmsg;
  }
}

void Logger::info(const char *msg, ...)
{
  if ( log_level <= 1) {
    char* fmsg = new char[100];

    va_list ap;
    va_start(ap, msg);
    int reallen = vsnprintf(fmsg, 100, msg, ap);
    if(reallen > 100){
      delete[] fmsg;
      fmsg = new char[reallen + 1];
      va_end(ap);
      va_start(ap, msg);
      vsnprintf(fmsg, reallen + 1, msg, ap);
    }
    va_end(ap);
    doLogging(1, fmsg);
    delete[] fmsg;
  }
}

void Logger::warning(const char *msg, ...)
{
  if ( log_level <= 2) {
    char* fmsg = new char[100];

    va_list ap;
    va_start(ap, msg);
    int reallen = vsnprintf(fmsg, 100, msg, ap);
    if(reallen > 100){
      delete[] fmsg;
      fmsg = new char[reallen + 1];
      va_end(ap);
      va_start(ap, msg);
      vsnprintf(fmsg, reallen + 1, msg, ap);
    }
    va_end(ap);
    doLogging(2, fmsg);
    delete[] fmsg;
  }
}

void Logger::error(const char *msg, ...)
{
  if ( log_level <= 3) {
    char* fmsg = new char[100];

    va_list ap;
    va_start(ap, msg);
    int reallen = vsnprintf(fmsg, 100, msg, ap);
    if(reallen > 100){
      delete[] fmsg;
      fmsg = new char[reallen + 1];
      va_end(ap);
      va_start(ap, msg);
      vsnprintf(fmsg, reallen + 1, msg, ap);
    }
    va_end(ap);
    doLogging(3, fmsg);
    delete[] fmsg;
  }
  //exit(1);
}


int Logger::addLog(LogSink* newlog)
{
  std::ostringstream extname;

  extname << "ext" << sink_count;
  sink_map[extname.str()] = newlog;
  return sink_count++;
}

void Logger::removeLog(int extid)
{
  std::ostringstream extname;

  extname << "ext" << extid;
  if (sink_map.find(extname.str()) != sink_map.end()) {
    delete sink_map[extname.str()];
    sink_map.erase(extname.str());
  }
}


void Logger::flush()
{
  info("Logger stopped");
}

void Logger::reconfigure(const std::string & item, const std::string & value)
{

  // Default log level to 0 (in case log_level is not present in config file)
  log_level = atoi(Settings::getSettings()->get("log_level").c_str());

  if (Settings::getSettings()->get("log_console") == "yes") {
    if (sink_map.find("console") == sink_map.end())
      sink_map["console"] = new ConsoleLogger();
  } else {
    if (sink_map.find("console") != sink_map.end()) {
      delete sink_map["console"];
      sink_map.erase("console");
    }
  }

  if (Settings::getSettings()->get("log_file") == "yes" && Settings::getSettings()->get("logfile_name") != ""){
    if ( sink_map.find( "file") == sink_map.end()) {
      try {
        sink_map["file"] = new FileLogger(
            Settings::getSettings()->get("logfile_name"));
      } catch (std::exception e){
        error("Could not start file log sink, could not open file");
      }
    }
  } else {
    if ( sink_map.find( "file") != sink_map.end()) {
      delete sink_map["file"];
      sink_map.erase("file");
    }
  }

  if ( Settings::getSettings()->get("log_syslog") == "yes") {
    if ( sink_map.find( "sys") == sink_map.end())
      sink_map["sys"] = new SysLogger();
  } else {
    if ( sink_map.find( "sys") != sink_map.end()) {
      delete sink_map["sys"];
      sink_map.erase("sys");
    }
  }
}

Logger::Logger() : sink_count(0)
{
  reconfigure("","");
  info("Logger started");
  Settings::getSettings()->setCallback("log_level",   boost::bind( &Logger::reconfigure, this, _1, _2 ));
  Settings::getSettings()->setCallback("log_console", boost::bind( &Logger::reconfigure, this, _1, _2 ));
  Settings::getSettings()->setCallback("log_syslog",  boost::bind( &Logger::reconfigure, this, _1, _2 ));
  Settings::getSettings()->setCallback("log_file",    boost::bind( &Logger::reconfigure, this, _1, _2 ));
}

Logger::~Logger()
{

}


void Logger::doLogging(int level, const char *msg)
{
  for (SinkMap::iterator pos = sink_map.begin(); pos != sink_map.end(); ++pos) {
    pos->second->doLogging(level, msg);
  }
}
