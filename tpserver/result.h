#ifndef RESULT_H
#define RESULT_H
/*  Result class for returning an error with a message
 *
 *  Copyright (C) 2004-2005, 2007  Tim Ansell and the Thousand Parsec Project
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

#define Success() Result(true, "")
#define Failure(msg) Result(false, msg)

class Result {
 public:
  Result(bool b, std::string message) {
    r = b;
    msg = std::string (message);
  }

  Result(bool b, const char* message) {
    r = b;
    msg = std::string (message);
  }

  operator bool() const {
    return r;
  }

  operator const char*() const {
    return msg.c_str();
  }

  friend std::string operator +(std::string l, Result r) {
    return l + r.msg;
  }

  friend std::string operator +(Result l, std::string r) {
	return l.msg + r;
  }

/*  ~Result() { 
    delete msg;
  } */

 private:
   std::string msg;
   bool r;
};

/*
int main() {
  if (!Failure("testing"))
     printf("Yay\n");
  printf("%s\n", (std::string("Hello") + Failure("Testing")).c_str());

} */

#endif
