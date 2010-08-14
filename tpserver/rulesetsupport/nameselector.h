#ifndef RULESETSUPPORT_NAMESELECTOR_H
#define RULESETSUPPORT_NAMESELECTOR_H
/*  NameSelector class
 *
 *  Copyright (C) 2010  Lee Begg and the Thousand Parsec Project
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

#include <set>
#include <stdint.h>
#include <iostream>

class NameSelector{
    public:
        NameSelector(const std::string& defaultname);
        virtual ~NameSelector();
        
        virtual std::string getName();
        
    private:
        uint32_t items;
        std::string prefix;
};


/**
 * Use a list of names from a file then fall back to "defaultname xx" names.
 */
class NamesFile : public NameSelector {
    public:
        NamesFile(std::istream* f, const std::string& defaultname);

        ~NamesFile();

        std::string getName();
  
    private:
        std::istream* m_file;

        /**
        * Read a line from a stream.
        */ 
        std::string readline();
};

class Random;

/**
 * Use a predefined list of names given and then fall back to "defaultprefix xx" names.
 */
class NamesSet : public NameSelector {
    public:
        NamesSet(Random* r, char const * const defaultNames[], size_t size, bool withreplacement, const std::string& defaultprefix);

        std::string getName();
  
    private:
        std::set<const char*> names;
        Random* rand;
        bool replace;
};

#endif