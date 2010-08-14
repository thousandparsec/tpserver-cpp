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

#include <sstream>
#include <cassert>

#include "prng.h"

#include "nameselector.h"

/**
 * Base class for various ways to get names for starsystems.
 */

NameSelector::NameSelector(const std::string& defaultname) : items(0), prefix(defaultname) {

}

/**
 * Get a name which is "prefix xx".
 */
std::string NameSelector::getName() {
  std::ostringstream name;
  name.str("");
  name << prefix << " " << ++items;

  return name.str();
}

NameSelector::~NameSelector() {};

NamesSet::NamesSet(Random* r, char const * const defaultNames[], size_t size, bool withreplacement, const std::string& defaultprefix) :
    NameSelector(defaultprefix),
    names(defaultNames, defaultNames + size),
    replace(withreplacement)
  {
    rand  = r;
  }

std::string NamesSet::getName() {
    if (names.size() > 0) {
        // Choose a random name
        uint32_t choice = rand->getInRange((uint32_t)0U, (uint32_t)names.size() - 1);

        std::set<const char*>::iterator name = names.begin();
        advance(name, choice);
        assert(name != names.end());

        if(!replace){
            names.erase(name);
        }

        return std::string(*name);
    } else {
        // Opps we ran out of precreated names!
        return NameSelector::getName();
    }
}


// FIXME: These belong in some type of string helper file
#define WHITESPACE " \t\f\v\n\r"

/**
 * Strip any trailing or leading characters of a given type
 * 
 * @param str The string to strip (unmodified)
 * @param sep The chars types to strip
 * @return    A stripped string
 */
inline std::string strip(std::string const& str, char const* sep) {
    std::string::size_type first = str.find_first_not_of(sep);
    std::string::size_type last  = str.find_last_not_of(sep);
    if ( first == std::string::npos || last  == std::string::npos )
        return std::string("");

    return str.substr(first, (last-first)+1);
}
/**
 * Does a string start with another string?
 * 
 * @param str      The string to match against
 * @param starting What the string should start with.
 * @return Does the string start with starting?
 */
inline bool startswith(std::string const& str, std::string const& starting) {
    if (str.length() < starting.length())
        return false;

    return str.substr(0, starting.length()) == starting;
}


#define BUFFERSIZE 1024

/**
 * Read a line from a stream.
 */ 
std::string NamesFile::readline() {
    std::string buffer;    

    // Get the next line
    // Loop while the buffer is empty 
    while (buffer.size() == 0 || m_file->fail()) {
      // Temporary storage for the line
      char cbuffer[BUFFERSIZE];
      uint32_t cbuffer_amount = 0;

      m_file->getline(cbuffer, BUFFERSIZE);
      cbuffer_amount = m_file->gcount();      // The amount of data which was put in our buffer

      if (cbuffer_amount > 0)
        buffer.append(std::string(cbuffer, cbuffer_amount-1));

      // Have we reached the end of the file
      if (m_file->eof())
        break;
    }

    return buffer;
}


NamesFile::NamesFile(std::istream* f, const std::string& defaultname) : NameSelector(defaultname) {
    m_file = f;
}

NamesFile::~NamesFile() {
    delete m_file;
}

std::string NamesFile::getName() {
    while (!m_file->eof()) {
      // Choose a random name
      std::string s = strip(readline(), WHITESPACE);
      if (s.length() == 0)
        continue;

      return s;
    }

    // Opps we ran out of precreated names!
    return NameSelector::getName();
}
