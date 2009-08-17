/*  Component Importation
 *
 *  Copyright (C) 2009  Alan P. Laudicina and the Thousand Parsec Project
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

#include <string>
#include <map>
#include "xmlimport.h"
#include <tpserver/logging.h>
#include <iostream>
#include "tpserver/game.h"
#include "tpserver/designstore.h"
#include "tpserver/component.h"
#include "tpserver/property.h"

namespace MTSecRuleset {

xmlImport::xmlImport() {

}

void xmlImport::setFile(std::string filename) {
    this->xmlFile = filename;
}

bool xmlImport::importComps() {
    TiXmlDocument doc(xmlFile.c_str());
    if(!doc.LoadFile()){
        Logger::getLogger()->debug("Error: could not load components XML file");
        return false;
    }

    TiXmlElement* pElem;
    TiXmlElement* pChild;
    TiXmlHandle hDoc(&doc);
    TiXmlHandle hRoot(0);
    DesignStore *ds = Game::getGame()->getDesignStore();

    int count=0;    // item count

    pElem = hDoc.FirstChildElement("components").Element();
    if(!pElem) return false;

    hRoot = TiXmlHandle(pElem);

    for(pElem=hRoot.FirstChild("comp").Element(); pElem != NULL;
    pElem = pElem->NextSiblingElement())
    {
        TiXmlElement* pCur = 0;

        std::string compName, compDescription, compTpcl, compIDName;
        std::map<uint32_t, std::string> propertylist;

        pChild = hRoot.Child("comp",count).Element();
        //debug: cout << "count: " << count << endl;
        if(pChild) { 
            //read and set the name of the component
            pCur = pChild->FirstChildElement("name");
            if (pCur) { 
                compName = pCur->GetText();
                if (compName.empty()) return false;
            } else {
                return false;
            }
            //read and set the ID of the component
            pCur = pChild->FirstChildElement("ComponentIDName");
            if (pCur) { 
                compIDName = pCur->GetText();
                if (compIDName.empty()) return false;
            } else {
                return false;
            }
            //read and set the description of the component
            pCur = pChild->FirstChildElement("description");
            if (pCur) {
                compDescription = pCur->GetText();
                if (compDescription.empty()) return false;
            } else {
                return false;
            }
            //read and set the tpclRequirementsFunction of the component
            pCur = pChild->FirstChildElement("tpclRequirementsFunction");
            if (pCur) {
                compTpcl = pCur->GetText();
                if (compTpcl.empty()) return false;
            } else {
                return false;
            }

            //read and set the tpclRequirementsFunction of the component
            pCur = pChild->FirstChildElement("propertylist");
            if (pCur) {
            TiXmlElement* pElem;
            for(pElem=pCur->FirstChildElement(); pElem != NULL; pElem=pElem->NextSiblingElement())
            {
                std::string pKey=pElem->Value();
                std::string pText=pElem->GetText();
                if (!pKey.empty() && !pText.empty()) {
                    propertylist[ds->getPropertyByName(pKey)] = pText;
                } else {
                    return false;
                }
            }
            } else {
                return false;
            }
        //do the component
        Component* comp = new Component();
        comp->addCategoryId(ds->getCategoryByName(compIDName));
        comp->setName(compName);
        comp->setDescription(compDescription);
        comp->setTpclRequirementsFunction(compTpcl);
        comp->setPropertyList(propertylist);
        ds->addComponent(comp);
        } else {
            return false;
        }
        count++;
    }
    return true;
}

bool xmlImport::importProps() {
    TiXmlDocument doc(xmlFile.c_str());
    if(!doc.LoadFile()){
        Logger::getLogger()->debug("Error: could not load properties XML file");
        return false;
    }

    TiXmlElement* pElem;
    TiXmlElement* pChild;
    TiXmlHandle hDoc(&doc);
    TiXmlHandle hRoot(0);
    DesignStore *ds = Game::getGame()->getDesignStore();

    int count=0;    // item count

    pElem = hDoc.FirstChildElement("properties").Element();
    if(!pElem) return false;

    hRoot = TiXmlHandle(pElem);

    for(pElem=hRoot.FirstChild("prop").Element(); pElem != NULL;
    pElem = pElem->NextSiblingElement())
    {
        TiXmlElement* pCur = 0;

        std::string propName, propDisplayName, propDescription, propTpclDisplay,
                    propTpclRequirement, propIDName;
        int propRank;
        std::map<uint32_t, std::string> propertylist;


        pChild = hRoot.Child("prop",count).Element();
        //debug: cout << "count: " << count << endl;
        if(pChild) { 
            //read and set the name of the property
            pCur = pChild->FirstChildElement("name");
            if (pCur) { 
                propName = pCur->GetText();
                if (propName.empty()) return false;
            } else {
                return false;
            }
            //read and set the name of the property
            pCur = pChild->FirstChildElement("displayName");
            if (pCur) { 
                propDisplayName = pCur->GetText();
                if (propDisplayName.empty()) return false;
            } else {
                return false;
            }
            //read and set the ID of the property
            pCur = pChild->FirstChildElement("PropertyIDName");
            if (pCur) { 
                propIDName = pCur->GetText();
                if (propIDName.empty()) return false;
            } else {
                return false;
            }
            //read and set the rank of the property
            pCur = pChild->FirstChildElement("rank");
            if (pCur) {
                if (pCur->QueryIntAttribute("value", &(propRank)) != TIXML_SUCCESS) {
                    return false;
                }
            }
            //read and set the description of the property
            pCur = pChild->FirstChildElement("description");
            if (pCur) {
                propDescription = pCur->GetText();
                if (propDescription.empty()) return false;
            } else {
                return false;
            }
            //read and set the tpclDisplayFunction of the property
            pCur = pChild->FirstChildElement("tpclDisplayFunction");
            if (pCur) {
                propTpclDisplay = pCur->GetText();
                if (propTpclDisplay.empty()) return false;
            } else {
                return false;
            }
            //read and set the tpclRequirementsFunction of the property
            pCur = pChild->FirstChildElement("tpclRequirementsFunction");
            if (pCur) {
                propTpclRequirement = pCur->GetText();
                if (propTpclRequirement.empty()) return false;
            } else {
                return false;
            }

        //do the property
            Property* prop = new Property();
            prop->addCategoryId(ds->getCategoryByName(propIDName));
            prop->setRank((uint32_t)propRank);
            prop->setName(propName);
            prop->setDisplayName(propDisplayName);
            prop->setDescription(propDescription);
            prop->setTpclDisplayFunction(propTpclDisplay);
            prop->setTpclRequirementsFunction(propTpclRequirement);
            ds->addProperty(prop);
        } else {
            return false;
        }
        count++;
    }
    return true;
}

}