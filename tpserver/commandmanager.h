#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H
/*  CommandManager class
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

#include <tpserver/inputframe.h>
#include <tpserver/outputframe.h>

class Command;

class CommandManager{
  public:
    static CommandManager *getCommandManager();

    bool checkCommandType(uint32_t type);
    void describeCommand(uint32_t cmdtype, OutputFrame * of);
    void addCommandType(Command* cmd);
    void doGetCommandTypes(InputFrame * frame, OutputFrame * of);
    void executeCommand(InputFrame * frame, OutputFrame * of);

  private:
    CommandManager();
    virtual ~CommandManager();

    std::map<uint32_t, Command*> commandStore;
    uint32_t nextType;

    uint32_t seqkey;

    static CommandManager *myInstance;

};

#endif
