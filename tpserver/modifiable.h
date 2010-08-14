#ifndef MODIFIABLE_H
#define MODIFIABLE_H

/*  Modifiable trait class
 *
 *  Copyright (C) 2009 Kornel Kisielewicz and the Thousand Parsec Project
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

#include <stdint.h>
#include <time.h>

class Modifiable {
public:
  Modifiable() { mod_time = 1; }
  virtual uint64_t getModTime() const { return mod_time; }
  virtual void touchModTime() { mod_time = mod_time+1; dirty = true; }
  void setModTime( uint64_t new_mod_time ) { mod_time = new_mod_time; }
  virtual bool isDirty() { return dirty; }
  virtual void setIsDirty( bool new_dirty ) { dirty = new_dirty; }
private:
  uint64_t mod_time;
  bool dirty;
};

#endif // MODIFIABLE_H

