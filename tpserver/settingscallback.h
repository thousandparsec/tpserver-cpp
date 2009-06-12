/*  SettingsCallback class/template
 *
 *  Copyright (C) 2006  Lee Begg and the Thousand Parsec Project
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
#ifndef SETTINGSCALLBACK_H
#define SETTINGSCALLBACK_H
// TODO: refactor this using boost::function


/** Baseclass for SettingsCallback

*/
class SettingsCallbackBase{
  public:
    SettingsCallbackBase(): ref(0) {}
    virtual ~SettingsCallbackBase(){};
    virtual void call(const std::string & name, const std::string & value) = 0;
    int ref;
};

/** A simple callback for Setting changes
	@author Lee Begg <llnz@paradise.net.nz>
*/
template<typename C , typename M>
    class SettingsCallbackImpl;

template<typename C , typename M>
class SettingsCallbackImpl : public SettingsCallbackBase
{
public:
  SettingsCallbackImpl(C const&objPtr, M mem_ptr)
  : SettingsCallbackBase(), object (objPtr), method (mem_ptr){};

  virtual ~SettingsCallbackImpl(){};
  
  virtual void call(const std::string & name, const std::string & value){
    ((*object).*method)(name, value);
  }
  
private:
  C const object;
  M method;

};



class SettingsCallback{
  public:
  template <typename OBJ_PTR, typename MEM_PTR>
    SettingsCallback (OBJ_PTR const &objPtr, MEM_PTR mem_ptr)
        : impl (new SettingsCallbackImpl<OBJ_PTR,MEM_PTR> (objPtr, mem_ptr)){
      impl->ref++;
    }
    
    SettingsCallback(const SettingsCallback& rhs){
      impl = rhs.impl;
      if(impl != NULL)
        impl->ref++;
    }
    
    SettingsCallback(): impl(NULL) {};
    
    ~SettingsCallback(){
      if(impl != NULL){
        impl->ref--;
        if(impl->ref == 0)
          delete impl;
      }
    }
    
    SettingsCallback operator=(const SettingsCallback & rhs){
      if(impl != NULL){
        impl->ref--;
        if(impl->ref == 0)
          delete impl;
      }
      impl = rhs.impl;
      impl->ref++;
      return *this;
    }
    
    void call(const std::string & name, const std::string & value){
      impl->call(name, value);
    }
  
  private:
    SettingsCallbackBase * impl;
};

#endif
