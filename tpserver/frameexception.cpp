/*  Frame Exception class
 *
 *  Copyright (C) 2010 Lee Begg and the Thousand Parsec Project
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


#include "frameexception.h"

FrameException::FrameException( FrameErrorCode code, const std::string& arg, const RefList& reflist ) : error_code(code), error_message(arg), error_reflist(reflist){
}

const char* FrameException::what() const throw(){
    return error_message.c_str();
}

FrameErrorCode FrameException::getErrorCode() const{
    return error_code;
}

const std::string& FrameException::getErrorMessage() const{
    return error_message;
}

const RefList& FrameException::getRefList() const{
    return error_reflist;
}

FrameException::~FrameException() throw(){
}

