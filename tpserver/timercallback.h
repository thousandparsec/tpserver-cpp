/*  TimerCallback class/template
 *
 *  Copyright (C) 2007  Lee Begg and the Thousand Parsec Project
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
#ifndef TIMERCALLBACK_H
#define TIMERCALLBACK_H

#include <ctime>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

/** Baseclass for TimerCallback */

class TimerCallback{
  public:
    /// typedef for callback function
    typedef boost::function< void() > Callback;
    /// typedef for shared pointer
    typedef boost::shared_ptr< TimerCallback > Ptr;

    TimerCallback (Callback acallback, uint32_t sec) : callback(acallback) {
      expiretime = sec + time(NULL);
    }
    
    TimerCallback(): callback(NULL), expiretime(0) {};
    
    uint64_t getExpireTime() const{
      return expiretime;
    }
    
    bool isValid() const{
      return (callback != NULL);
    }
    
    void invalidate() {
      callback = NULL;
    }
    
    void call() const {
      callback();
    }
  
  private:
    Callback callback;
    uint64_t expiretime;
};

#endif
