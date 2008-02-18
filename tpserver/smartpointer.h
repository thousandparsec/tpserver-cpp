#ifndef SMARTPOINTER_H
#define SMARTPOINTER_H
/*  SmartPointer class
 *  Creates a smart pointer for a suitable object.
 *
 *  Copyright (C) 2007, 2008  Lee Begg and the Thousand Parsec Project
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

#include <stdint.h>
#include <string>
#include <list>

template <typename T> class SmartPointerImpl;

template <typename T> class SmartPointerImpl{
  public:
    uint32_t ref;
    T* data;
};

template <typename T> class SmartPointer;

template <typename T> class SmartPointer{
  public:
    SmartPointer(): data(NULL){
    }
    
    SmartPointer(const SmartPointer<T>& rhs){
      data = rhs.data;
      if(data != NULL){
        data->ref++;
      }
    }
    
    SmartPointer(T* nd){
      if(nd != NULL){
        data = new SmartPointerImpl<T>();
        data->ref = 1;
        data->data = nd;
      }
    }
    
    ~SmartPointer(){
      if(data != NULL){
        data->ref--;
        if(data->ref == 0){
          delete data->data;
          delete data;
        }
      }
    }
    
    SmartPointer<T>& operator=(const SmartPointer<T>& rhs){
      if(data != NULL){
        data->ref--;
        if(data->ref == 0){
          delete data->data;
          delete data;
        }
      }
      data = rhs.data;
      if(data != NULL){
        data->ref++;
      }
      return *this;
    }
    
    SmartPointer<T>& operator=(T* nd){
      if(data != NULL){
        data->ref--;
        if(data->ref == 0){
          delete data->data;
          delete data;
        }
      }
      if(nd == NULL){
        data = NULL;
      }else{
        data = new SmartPointerImpl<T>();
        data->ref = 1;
        data->data = nd;
      }
      return *this;
    }
    
    T* operator->() const{
      if(data != NULL){
        return data->data;
      }else{
        return NULL;
      }
    }
    
  private:
    SmartPointerImpl<T>* data;
};

#endif
