/*  Manages the plugins
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

#include <dlfcn.h>

#include "logging.h"
#include "settings.h"

#include "pluginmanager.h"

typedef bool (* tpinit_function)(void);

PluginManager *PluginManager::instance = NULL;

PluginManager::PluginManager() : libs()
{}


PluginManager::~PluginManager(){
  if(!(libs.empty())){
    stop();
  }
}


PluginManager *PluginManager::getPluginManager(){
  if (instance == NULL)
    instance = new PluginManager();
  return instance;
}

void PluginManager::start(){
  
}

void PluginManager::stop(){
  for(std::map<std::string, void*>::iterator itcurr = libs.begin(); itcurr != libs.end(); ++itcurr){
    dlclose(itcurr->second);
  }
  libs.clear();
}

bool PluginManager::load(const std::string& libname){
  Logger* log = Logger::getLogger();
  dlerror(); // clear errors
  void* nlib = dlopen(libname.c_str(), RTLD_NOW | RTLD_GLOBAL);
  if(nlib == NULL){
    log->error("Failed to load plugin \"%s\": \"%s\"", libname.c_str(), dlerror());
    return false;
  }else{
    log->debug("Loaded plugin \"%s\" sucessfully", libname.c_str());
    dlerror(); // clear errors
    tpinit_function init;
    *(void **) (&init)= dlsym(nlib, "tp_init");
    if(init == NULL){
      log->error("Failed to initialise plugin \"%s\": \"%s\"", libname.c_str(), dlerror());
      dlclose(nlib);
      return false;
    }else{
      log->debug("Initialisation function for plugin \"%s\" found", libname.c_str());
      if((*init)()){
        log->info("Loaded plugin \"%s\" sucessfully", libname.c_str());
        libs[libname] = nlib;
      }else{
        log->error("Could not initialise plugin \"%s\"", libname.c_str());
        dlclose(nlib);
        return false;
      }
    }
  }
  return true;
}

std::string PluginManager::getLoadedLibraryNames() const{
  return "";
}

