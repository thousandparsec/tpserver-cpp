#ifndef DATASTORE_H
#define DATASTORE_H
/* Storage and manage for Binary Data Chunks.
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

#include <map>

class Frame;
class DataChunk;

class DataStore {
 public:
  DataStore();
  ~DataStore();

  void getData(unsigned int dataid, Frame* frame, int pid);
  void removeData(unsigned int dataid, Frame* frame, int pid);

  void addData(DataChunk* dc);

 private:

  std::map<unsigned int, DataChunk*> store;

};

#endif
