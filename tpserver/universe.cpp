/*  Universe object
 *
 *  Copyright (C) 2003-2004  Lee Begg and the Thousand Parsec Project
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

#include "frame.h"

#include "universe.h"

Universe::Universe()
{
	yearNum = 0;
}

void Universe::packExtraData(Frame * frame)
{
	frame->packInt(yearNum);
}

void Universe::doOnceATurn(IGObject * obj)
{
	++yearNum;
        touchModTime();
}

int Universe::getContainerType(){
  return 1;
}

ObjectData* Universe::clone(){
  return new Universe();
}

void Universe::setYear(int year)
{
	yearNum = year;
}

int Universe::getYear()
{
	return yearNum;
}
