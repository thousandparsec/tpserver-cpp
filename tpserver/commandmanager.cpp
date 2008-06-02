/*  CommandManager for managing commands
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
#include "frame.h"

#include "commandmanager.h"

CommandManager *CommandManager::myInstance = NULL;

CommandManager *CommandManager::getCommandManager()
{
  if(myInstance == NULL)
    myInstance = new CommandManager();

  return myInstance;
}

bool CommandManager::checkCommandType(uint32_t type)
{
    return (type >= 0 && type < nextType);
}

void CommandManager::describeCommand(uint32_t cmdtype, Frame * f)
{
    if(commandStore.find(cmdtype) != commandStore.end()){
        commandStore[cmdtype]->describeCommand(f);
    }else{
        f->createFailFrame(fec_NonExistant, "Command type does not exist");
    }
}

void CommandManager::addCommandType(Command* cmd){
  cmd->setType(nextType);
  commandStore[nextType++] = cmd;
  seqkey++;
}

void CommandManager::doGetCommandTypes(Frame* frame, Frame * of){
  uint32_t lseqkey = frame->unpackInt();
  if(lseqkey == UINT32_NEG_ONE){
    //start new seqkey
    lseqkey = seqkey;
  }

  uint32_t start = frame->unpackInt();
  uint32_t num = frame->unpackInt();
  
  if(lseqkey != seqkey){
    of->createFailFrame(fec_TempUnavailable, "Invalid Sequence Key");
    return;
  }

  if(start > commandStore.size()){
    of->createFailFrame(fec_NonExistant, "Starting number too high");
    return;
  }
  
  if(num > commandStore.size() - start){
    num = commandStore.size() - start;
  }
  
  if(num > MAX_ID_LIST_SIZE + ((of->getVersion() < fv0_4) ? 1 : 0)){
    of->createFailFrame(fec_FrameError, "Too many items to get, frame too big");
    return;
  }

  of->setType(ftad_CommandTypes_List);
  of->packInt(lseqkey);
  of->packInt(commandStore.size() - start - num);
  of->packInt(num);
  std::map<uint32_t, Command*>::iterator itcurr = commandStore.begin();
  advance(itcurr, start);
  for(uint32_t i = 0; i < num; i++){
    of->packInt(itcurr->first);
    ++itcurr;
  }
}

CommandManager::CommandManager()
{
    nextType = 0;
}

CommandManager::~CommandManager()
{
    for(std::map<uint32_t, Command*>::iterator itcurr = commandStore.begin(); itcurr != commandStore.end(); ++itcurr){
      delete itcurr->second;
    }
}
