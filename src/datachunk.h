#ifndef DATACHUNK_H
#define DATACHUNK_H
/* Binary Data Chunks object.
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

#include <string>


class Frame;

class DataChunk{
 public:
  DataChunk();
  DataChunk(const DataChunk &rhs);
  ~DataChunk();

  unsigned int getId() const;
  unsigned int getOwner() const;

  void setOwner(unsigned int playerid);
  void setTurnCreated(unsigned int cturn);
  void setMimeType(const std::string mtype);
  void setData(unsigned int len, char* data); // data *is* copied.

  void packHeader(Frame* f) const;
  void packData(Frame* f) const;

 private:
  static unsigned int nextid;
  unsigned int did;
  unsigned int pid;
  unsigned int turn;
  std::string mimetype;
  unsigned int length;
  char* bdata;

};


#endif 
