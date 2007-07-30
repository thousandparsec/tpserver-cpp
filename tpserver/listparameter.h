#ifndef LISTPARAMETER_H
#define LISTPARAMETER_H
/*  ListParameter class
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

#include <tpserver/orderparameter.h>
#include <map>


class ListOptionCallbackBase{
  public:
    ListOptionCallbackBase(): ref(0) {}
    virtual ~ListOptionCallbackBase(){};
    virtual std::map<uint32_t, std::pair<std::string, uint32_t> > call() = 0;
    int ref;
};

/** A simple callback for ListOptions
	@author Lee Begg <llnz@paradise.net.nz>
*/
template<typename C , typename M>
    class ListOptionCallbackImpl;

template<typename C , typename M>
class ListOptionCallbackImpl : public ListOptionCallbackBase
{
public:
  ListOptionCallbackImpl(C const&objPtr, M mem_ptr)
  : ListOptionCallbackBase(), object (objPtr), method (mem_ptr){};

  virtual ~ListOptionCallbackImpl(){};
  
  virtual std::map<uint32_t, std::pair<std::string, uint32_t> > call(){
    return ((*object).*method)();
  }
  
private:
  C const object;
  M method;

};



class ListOptionCallback{
  public:
  template <typename OBJ_PTR, typename MEM_PTR>
    ListOptionCallback (OBJ_PTR const &objPtr, MEM_PTR mem_ptr)
        : impl (new ListOptionCallbackImpl<OBJ_PTR,MEM_PTR> (objPtr, mem_ptr)){
      impl->ref++;
    }
    
    ListOptionCallback(const ListOptionCallback& rhs){
      impl = rhs.impl;
      if(impl != NULL)
        impl->ref++;
    }
    
    ListOptionCallback(): impl(NULL) {};
    
    ~ListOptionCallback(){
      if(impl != NULL){
        impl->ref--;
        if(impl->ref == 0)
          delete impl;
      }
    }
    
    ListOptionCallback operator=(const ListOptionCallback & rhs){
      if(impl != NULL){
        impl->ref--;
        if(impl->ref == 0)
          delete impl;
      }
      impl = rhs.impl;
      impl->ref++;
      return *this;
    }
    
    std::map<uint32_t, std::pair<std::string, uint32_t> > call(){
      return impl->call();
    }
  
  private:
    ListOptionCallbackBase * impl;
};



class ListParameter : public OrderParameter{

public:
  ListParameter();
  virtual ~ListParameter();

  virtual void packOrderFrame(Frame * f);
  virtual bool unpackFrame(Frame * f, unsigned int playerid);

  virtual OrderParameter *clone() const;

  std::map<uint32_t,uint32_t> getList() const;
  void setList(std::map<uint32_t,uint32_t> nlist);
  
  void setListOptionsCallback(ListOptionCallback cb);

protected:
  std::map<uint32_t,uint32_t> list;
  ListOptionCallback optionscallback;

};

#endif
