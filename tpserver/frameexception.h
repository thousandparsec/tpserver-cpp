#ifndef FRAMEEXCEPTION_H
#define FRAMEEXCEPTION_H
/*  Frame Exception class
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

#include <exception>
#include <string>
#include <tpserver/protocol.h>
#include <tpserver/common.h>

/**
 * Frame exception class
 *
 * Except for storing the error string that is later to be sent as a reply
 * it also stores the error code.
 */
class FrameException : public std::exception {
  public:
    /**
     * Creation of exception
     *
     * @param code Frame error code as per protocol.h
     * @param arg Optional error string
     */

    explicit FrameException( FrameErrorCode code, const std::string& arg = "", const RefList& reflist = RefList() );

    /**
     * Standard what() method, returns formated error message with
     * descriptive error_code definition.
     */
    virtual const char* what() const throw();
    /**
     * Returns error code
     */
    FrameErrorCode getErrorCode() const;
    /**
     * Returns error message only
     */
    const std::string& getErrorMessage() const;
    
    /**
     * Returns RefList
     */
    const RefList& getRefList() const;
    
    /**
     * Standard destructor, does nothing.
     */
    virtual ~FrameException() throw();

  private:
    /// Stored error code
    FrameErrorCode  error_code;

    /// Stored error message
    std::string     error_message;
    
    /// Stored RefList
    RefList error_reflist;
};

#endif
