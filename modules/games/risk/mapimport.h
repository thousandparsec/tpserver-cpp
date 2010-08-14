#ifndef MAPIMPORT_H
#define MAPIMPORT_H
/*  mapimport class
 *
 *  Copyright (C) 2008  Ryan Neufeld and the Thousand Parsec Project
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
 
#include <tpserver/object.h>
#include <tpserver/vector3d.h>

#include <wormhole.h>

#include <tpserver/tinyxml/tinyxml.h>

#include <vector>
#include <string>
#include <map>
class IGObject;
class NameSelector;

namespace RiskRuleset {

    class MapImport{
        public:
            MapImport();
            
            bool importMapFromFile(std::string filename, IGObject& universe);
            bool processGTag(TiXmlElement* pG, IGObject& universe);
            bool processRectTag(TiXmlElement* pRect, IGObject& universe, std::map<std::string,IGObject::Ptr >& labelToPlanet);
            std::pair<std::string,uint32_t> getNameAndBonus(TiXmlElement* pG, std::string fill);
            std::string removeUnderscores(std::string str);

            std::string getFillFromStyle(std::string longStyle);
            IGObject::Ptr createConstellation(IGObject& parent, const std::string& name, int bonus);
            IGObject::Ptr createStarSystem(IGObject& parent, const std::string& name,
               double unitX, double unitY);
            IGObject::Ptr createPlanet(IGObject& parent, const std::string& name,
               double unitX, double unitY);                        
            IGObject::Ptr createPlanet(IGObject& parentStarSys, const std::string& name,
               const Vector3d& location);
            void createWormhole(IGObject& parent, int64_t startat, int64_t endat);
            void createWormhole(IGObject& parent, IGObject::Ptr startat, IGObject::Ptr endat);
            
        private:
            NameSelector* constellationmedia;
            NameSelector* systemmedia;
            NameSelector* planetmedia;

    };
    
}

#endif
