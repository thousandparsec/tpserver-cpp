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

#include <ltdl.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#ifndef LIBDIR
#define LIBDIR "/usr/local/lib"
#endif
#endif

#ifdef USE_LIBTOOL2 
# define lt_preloaded_symbols lt__PROGRAM__LTX_preloaded_symbols 
#endif

#include "logging.h"
#include "settings.h"

#include "pluginmanager.h"

extern const lt_dlsymlist lt_preloaded_symbols[];

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
  Logger* log = Logger::getLogger();

  std::string list = Settings::getSettings()->get("autoload_plugins");
  size_t pos_c = 0;
  size_t pos_e;
  if(list.length() > 0){
    while(pos_c != list.npos){
      pos_e = list.find(",", pos_c);
      load(list.substr(pos_c, pos_e));
      pos_c = pos_e;
    }
  }else{
    log->info("No automatically loaded plugins were defined in the "
        "configuation, add \"autoload_plugins = <comma list of plugins>\" to conf "
        "to have them loaded.");
  }

  /* Initalise the ltdl module. */
  LTDL_SET_PRELOADED_SYMBOLS();
  if(lt_dlinit() != 0){
    // FIXME: Should raise an error here.!
    log->error("Failed to load initalise the loader %s", lt_dlerror());
	stop();
  }

  log->info("Started looking for preloaded modules.");
  for (int i = 0; lt_preloaded_symbols[i].name != NULL; i++) {
    int len = strlen(lt_preloaded_symbols[i].name);

    if (strncmp(lt_preloaded_symbols[i].name, "lib", 3) == 0 && 
        strncmp(lt_preloaded_symbols[i].name+(len-2), ".a", 2) == 0)
      log->info("Found %s at %p", lt_preloaded_symbols[i].name, lt_preloaded_symbols[i].address);
  }
  log->info("Finished looking for preloaded modules.");


  /* Need to load ourselves before we can load other modules. */
  lt_dlhandle nlib = lt_dlopen(NULL);
  if(nlib == NULL){
    log->error("Failed to load ourselves because %s.", lt_dlerror());
	stop();
  }

}

void PluginManager::stop(){
  for(std::map<std::string, void*>::iterator itcurr = libs.begin(); itcurr != libs.end(); ++itcurr){
    lt_dlclose((lt_dlhandle)itcurr->second);
  }
  libs.clear();
}

bool PluginManager::load(const std::string& libname){
  Logger* log = Logger::getLogger();

  lt_dlhandle nlib = lt_dlopenext(libname.c_str());
  if(nlib == NULL){
    log->error("Failed to load plugin \"%s\": \"%s\" %p", libname.c_str(), lt_dlerror(), nlib);
    return false;
  }else{
    log->debug("Loaded plugin \"%s\" sucessfully", libname.c_str());
    tpinit_function init;
    *(void **) (&init)= lt_dlsym(nlib, "tp_init");
    if(init == NULL){
      log->error("Failed to initialise plugin \"%s\": \"%s\"", libname.c_str(), lt_dlerror());
      lt_dlclose(nlib);
      return false;
    }else{
      log->debug("Initialisation function for plugin \"%s\" found", libname.c_str());
      if((*init)()){
        log->info("Loaded plugin \"%s\" sucessfully", libname.c_str());
        libs[libname] = nlib;
      }else{
        log->error("Could not initialise plugin \"%s\"", libname.c_str());
        lt_dlclose(nlib);
        return false;
      }
    }
  }
  return true;
}

std::string PluginManager::getLoadedLibraryNames() const{
  std::string list;
  bool firsttime = true;
  for(std::map<std::string, void*>::const_iterator itcurr = libs.begin(); itcurr != libs.end(); ++itcurr){
    if(!firsttime){
      list.append(",");
    }else{
      firsttime = false;
    }
    list.append(itcurr->first);
  }
  return list;
}

bool PluginManager::loadRuleset(const std::string& name){
  if(name.find("/") == name.npos)
    return load(std::string(LIBDIR "/tpserver/ruleset/lib").append(name));
  else
    return load(name);
}

bool PluginManager::loadPersistence(const std::string& name){
  if(name.find("/") == name.npos)
    return load(std::string(LIBDIR "/tpserver/persistence/lib").append(name));
  else
    return load(name);
}

bool PluginManager::loadTpScheme(const std::string& name){
  if(name.find("/") == name.npos)
    return load(std::string(LIBDIR "/tpserver/tpscheme/lib").append(name));
  else
    return load(name);
}
