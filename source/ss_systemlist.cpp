//
// Created by cpasjuste on 13/12/2019.
//

#include <algorithm>
#include "ss_api.h"

using namespace ss_api;

SystemList::SystemList(const std::string &ssid, const std::string &sspassword, int retryDelay) {

    long code = 0;
    Curl ss_curl;
    std::string soft = ss_curl.escape(Api::ss_softname);
    std::string url = "https://www.screenscraper.fr/api2/systemesListe.php?devid="
                      + Api::ss_devid + "&devpassword=" + Api::ss_devpassword
                      + "&softname=" + soft + "&output=xml";

    url += ssid.empty() ? "" : "&ssid=" + ssid;
    url += sspassword.empty() ? "" : "&sspassword=" + sspassword;

    SS_PRINT("SystemList: %s\n", url.c_str());

    std::string xml = ss_curl.getString(url, SS_TIMEOUT, &code);
    if (retryDelay > 0) {
        while (code == 429 || code == 28) {
            Api::printe((int) code, retryDelay);
            Io::delay(retryDelay);
            xml = ss_curl.getString(url, SS_TIMEOUT, &code);
        }
    }

    if (code != 0 || xml.empty()) {
        SS_PRINT("SystemList: error %li\n", code);
        http_error = (int) code;
        return;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError e = doc.Parse(xml.c_str(), xml.size());
    if (e != tinyxml2::XML_SUCCESS) {
        SS_PRINT("SystemList: %s\n", doc.ErrorName());
        doc.Clear();
        return;
    }

    tinyxml2::XMLNode *pRoot = doc.FirstChildElement("Data");
    if (pRoot == nullptr) {
        SS_PRINT("SystemList: wrong xml format: \'Data\' tag not found\n");
        doc.Clear();
        return;
    }

    tinyxml2::XMLNode *systemNode = pRoot->FirstChildElement("systeme");
    while (systemNode != nullptr) {
        System system;
        parseSystem(&system, systemNode);
        systems.emplace_back(system);
        systemNode = systemNode->NextSibling();
    }

    doc.Clear();
}

bool SystemList::parseSystem(struct ss_api::SystemList::System *system, tinyxml2::XMLNode *systemNode) {

    if (system == nullptr || systemNode == nullptr) {
        return false;
    }

    system->id = Api::getXmlText(systemNode->FirstChildElement("id"));
    system->parentid = Api::getXmlText(systemNode->FirstChildElement("parentid"));
    system->extensions = Api::getXmlText(systemNode->FirstChildElement("extensions"));
    system->company = Api::getXmlText(systemNode->FirstChildElement("compagnie"));
    system->type = Api::getXmlText(systemNode->FirstChildElement("type"));
    system->startdate = Api::getXmlText(systemNode->FirstChildElement("datedebut"));
    system->enddate = Api::getXmlText(systemNode->FirstChildElement("datefin"));
    system->romtype = Api::getXmlText(systemNode->FirstChildElement("romtype"));
    system->supporttype = Api::getXmlText(systemNode->FirstChildElement("supporttype"));
    tinyxml2::XMLNode *element = systemNode->FirstChildElement("noms");
    if (element != nullptr) {
        system->names.eu = Api::getXmlText(element->FirstChildElement("nom_eu"));
        system->names.recalbox = Api::getXmlText(element->FirstChildElement("nom_recalbox"));
        system->names.retropie = Api::getXmlText(element->FirstChildElement("nom_retropie"));
        system->names.launchbox = Api::getXmlText(element->FirstChildElement("nom_launchbox"));
        system->names.hyperspin = Api::getXmlText(element->FirstChildElement("nom_hyperspin"));
        system->names.common = Api::getXmlText(element->FirstChildElement("noms_commun"));
    }

    return true;
}

SystemList::System SystemList::findById(const std::string &id) {

    auto it = std::find_if(systems.begin(), systems.end(), [id](const System &system) {
        return system.id == id;
    });

    if (it != systems.end()) {
        return *it;
    }

    return System();
}
