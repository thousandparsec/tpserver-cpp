/*  Logging for tpserver-cpp
 *
 *  Copyright (C) 2003-2005  Lee Begg and the Thousand Parsec Project
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

#include <syslog.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "syslogger.h"

SysLogger::SysLogger()
{
    openlog( "tpserver-cpp", LOG_PID, LOG_DAEMON);
}

SysLogger::~SysLogger()
{
    closelog();
}

void SysLogger::reconfigure()
{
}

void SysLogger::doLogging( int level, char* msg) const
{
    switch ( level) {
    case 0:
        level = LOG_DEBUG;
        break;
    case 1:
        level = LOG_INFO;
        break;
    case 2:
        level = LOG_WARNING;
        break;
    case 3:
        level = LOG_ERR;
        break;
    default:
        level = LOG_NOTICE;
    }

    syslog( level, msg);
}
