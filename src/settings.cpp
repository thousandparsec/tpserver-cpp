/*  Setting class
 *
 *  Copyright (C) 2003-2004  Lee Begg and the Thousand Parsec Project
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

#include <cassert>

#include "settings.h"

Settings *Settings::myInstance = NULL;

Settings *Settings::getSettings()
{
	if (myInstance == NULL) {
		myInstance = new Settings();
		myInstance->setDefaultValues();
	}
	return myInstance;
}

bool Settings::readArgs(int arg, char** argv){

  return true;
}

/*
void Settings::set(int item, int value){

}

void Settings::set(int item, String value){

}

void set(int item, bool value){
  
}

int Settings::get(int item){
  return -0;
}

String Settings::get(int item){
  return "";
}

bool Settings::get(int item){
  return false;
}
*/

Settings::Settings()
{

}

Settings::~Settings()
{

}

Settings::Settings(Settings & rhs)
{
}

Settings Settings::operator=(Settings & rhs)
{
  //please don't call me
  assert(0);
  return *this;
}

void Settings::setDefaultValues(){

}
