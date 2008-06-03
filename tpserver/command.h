#ifndef COMMAND_H
#define COMMAND_H
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

#include <list>
#include <string>

typedef enum {
    cpT_Invalid = -1,
    cpT_String = 0,
    cpT_Integer = 1,
    cpT_Max
} CommandParamType;

class Frame;

class CommandParameter {
  public:
    CommandParameter(uint32_t cpt, const char* cpn, const char* cpd);
    virtual ~CommandParameter();

    uint32_t getType() const;
    std::string getName() const;
    std::string getDescription() const;

    void packCommandDescFrame(Frame * of) const;

  protected:
    uint32_t type;
    std::string name;
    std::string description;

};

class Command {
  public:
    Command();
    virtual ~Command();

    uint32_t getType() const;
    void setType(uint32_t ntype);

    std::string getName() const;
    std::string getHelp() const;
    std::list<CommandParameter*> getParameters() const;

    void describeCommand(Frame * of) const;

    virtual void action(Frame * frame, Frame * of) = 0;

  protected:
    void addCommandParameter(CommandParameter* cp);

    uint32_t type;
    std::string name;
    std::string help;
  
  private:
    std::list<CommandParameter*> parameters;

};

#endif
