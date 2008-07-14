/*  MapImport
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
 *  Copyright (C) 2007  Tyler Shaub and the Thousand Parsec Project
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
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
#define TIXML_USE_TICPP

#include <tpserver/logging.h>

#include "tinyxml.h"
#include "mapimport.h"

#include <string>

namespace RiskRuleset {

using std::string;

bool importMapFromFile(string filename){
   bool loadedMapOkay = false;
   Logger::getLogger()->debug("Trying to load:  %s", filename.c_str());
   TiXmlDocument map(filename.c_str());  //Create a new xml doc object from filename
   loadedMapOkay = map.LoadFile();   //Load that file, recording success in loadedMap

   if (loadedMapOkay) {              //if - loading map was successful, we create all elements from the map
      Logger::getLogger()->debug("Loaded %s", filename.c_str());

      TiXmlElement *pRoot, *pG, *pRect, *pPath;

      //Get the root svg element
      pRoot = map.FirstChildElement("svg");
      if ( pRoot ) {

         //Get the root g element
         pG = pRoot->FirstChildElement("g");

         if (pG) {
            //Start processing Rectangles in g
            pRect = pG->FirstChildElement("rect");
            Logger::getLogger()->debug("Starting on rects processing");
            while (pRect) {
               //Process each individual Rectangle and translate to Star
               Logger::getLogger()->debug("Got rect, id is: %s",pRect->Attribute("id"));

               //Get the next rect
               pRect = pRect->NextSiblingElement("rect");
            }
            Logger::getLogger()->debug("Finished with rects processing");

            //Start processing Paths in g
            pPath = pG->FirstChildElement("path");
            Logger::getLogger()->debug("Starting on paths processing");
            while (pPath) {
               //Process each individual Path and translate into graph
               Logger::getLogger()->debug("Looking at a path");

               //Get the next path
               pPath = pPath->NextSiblingElement("path");
            }
            Logger::getLogger()->debug("Finished with paths processing");
         }// </g>
      }// </svg>

   }

   return loadedMapOkay;             //return true if map was created successfully, otherwise false
}

}//end namespace RiskRuleset
