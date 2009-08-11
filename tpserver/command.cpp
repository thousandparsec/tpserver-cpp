/*  Command class for remote administration
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

#include "command.h"
#include <boost/bind.hpp>

CommandParameter::CommandParameter(uint32_t cpt, const char* cpn, const char* cpd) : type(cpt)
{
    name = cpn;
    description = cpd;
}

CommandParameter::~CommandParameter()
{
}

uint32_t CommandParameter::getType() const
{
    return type;
}

std::string CommandParameter::getName() const
{
    return name;
}

std::string CommandParameter::getDescription() const
{
    return description;
}

void CommandParameter::packCommandDescFrame(OutputFrame * of) const
{
    of->packString(name);
    of->packInt(type);
    of->packString(description);
}

Command::Command() : name(), help()
{
}

Command::~Command()
{
}

uint32_t Command::getType() const
{
    return type;
}

void Command::setType(uint32_t ntype)
{
    type = ntype;
}

void Command::setDescriptionModTime(uint64_t mtime)
{
    descmodtime = mtime;
}

std::string Command::getName() const
{
    return name;
}

std::string Command::getHelp() const
{
    return help;
}

std::list<CommandParameter*> Command::getParameters() const
{
    return parameters;
}

uint64_t Command::getDescriptionModTime() const
{
    return descmodtime;
}

void Command::describeCommand(OutputFrame * of) const
{
    of->setType(ftad_CommandDesc);
    of->packInt(type);
    of->packString(name);
    of->packString(help);
    of->packInt(parameters.size());
    std::for_each( parameters.begin(), parameters.end(), boost::bind( &CommandParameter::packCommandDescFrame, _1, of ) );
    of->packInt64(descmodtime);
}

void Command::addCommandParameter(CommandParameter* cp)
{
    parameters.push_back(cp);
}
