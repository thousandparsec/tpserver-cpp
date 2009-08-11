/*  Frame class, the network packets for the TP procotol
 *
 *  Copyright (C) 2003-2005, 2007, 2008  Lee Begg and the Thousand Parsec Project
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

// Default to creating version 3 frames
Frame::Frame(ProtocolVersion v)
  : version(v), type(ft_Invalid), typeversion(0),
    sequence(0), padstrings(false)
{
}

Frame::~Frame()
{
}

FrameType Frame::getType() const
{
  return type;
}

int Frame::getSequence() const
{
  return sequence;
}

ProtocolVersion Frame::getVersion() const
{
  return version;
}

int Frame::getLength() const
{
  return getHeaderLength()+getDataLength();
}

int Frame::getHeaderLength() const
{
  return 16;
}

int Frame::getDataLength() const
{
  return data.length();
}


uint32_t Frame::getTypeVersion() const{
  return typeversion;
}

bool Frame::isPaddingStrings() const{
  return padstrings;
}

