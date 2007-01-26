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

/** Baseclass for TimerCallback

*/
class TimerCallbackBase{
  public:
    TimerCallbackBase(): ref(0), valid(true) {}
    virtual ~TimerCallbackBase(){};
    virtual void call() const = 0;
    int ref;
    bool valid;
};

/** A simple callback for Timer
	@author Lee Begg <llnz@paradise.net.nz>
*/
template<typename C , typename M>
    class TimerCallbackImpl;

template<typename C , typename M>
class TimerCallbackImpl : public TimerCallbackBase
{
public:
  TimerCallbackImpl(C const&objPtr, M mem_ptr)
  : TimerCallbackBase(), object (objPtr), method (mem_ptr){};

  virtual ~TimerCallbackImpl(){};
  
  virtual void call() const{
    ((*object).*method)();
  }
  
private:
  C const object;
  M method;

};



class TimerCallback{
  public:
  template <typename OBJ_PTR, typename MEM_PTR>
    TimerCallback (OBJ_PTR const &objPtr, MEM_PTR mem_ptr, uint64_t sec)
        : impl (new TimerCallbackImpl<OBJ_PTR,MEM_PTR> (objPtr, mem_ptr)), expiretime(sec + time(NULL)) {
      impl->ref++;
    }
    
    TimerCallback(const TimerCallback& rhs){
      impl = rhs.impl;
      if(impl != NULL)
        impl->ref++;
      expiretime = rhs.expiretime;
    }
    
    TimerCallback(): impl(NULL), expiretime(0) {};
    
    ~TimerCallback(){
      if(impl != NULL){
        impl->ref--;
        if(impl->ref == 0)
          delete impl;
      }
    }
    
    uint64_t getExpireTime() const{
      return expiretime;
    }
    
    bool isValid() const{
      return (impl != NULL) ? impl->valid : false;
    }
    
    void setValid(bool value){
      if(impl != NULL)
        impl->valid = value;
    }
    
    TimerCallback operator=(const TimerCallback & rhs){
      if(impl != NULL){
        impl->ref--;
        if(impl->ref == 0)
          delete impl;
      }
      impl = rhs.impl;
      impl->ref++;
      return *this;
    }
    
    bool operator>(const TimerCallback & rhs) const{
      return (expiretime > rhs.expiretime);
    }
    
    void call() const {
      impl->call();
    }
  
  private:
    TimerCallbackBase * impl;
    uint64_t expiretime;
};

#endif
