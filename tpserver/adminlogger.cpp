/*  Logging for tpserver-cpp
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

#include <time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "adminconnection.h"
#include "frame.h"

#include "adminlogger.h"

AdminLogger::AdminLogger() : connection(NULL) {
}

AdminLogger::~AdminLogger() {
}

void AdminLogger::setConnection(AdminConnection* newcon) {
    connection = newcon;
}

AdminConnection *AdminLogger::getConnection() const {
    return connection;
}

void AdminLogger::doLogging(int level, const char* msg) const {
    uint64_t timestamp = time(NULL);

    //TODO: assert(connection);

    if (connection->getStatus() == Connection::READY) {
        OutputFrame* logmessage = connection->createFrame(NULL);
        logmessage->setType(ftad_LogMessage);
        logmessage->packInt64(timestamp);
        logmessage->packInt(level);
        logmessage->packString(msg);
        connection->sendFrame(logmessage);
    }
}
