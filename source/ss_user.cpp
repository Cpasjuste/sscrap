//
// Created by cpasjuste on 11/12/2019.
//

#include "ss_api.h"

using namespace ss_api;

User::User(const std::string &ssid, const std::string &sspassword, int retryDelay) {

    long code = 0;
    Curl ss_curl;
    std::string soft = ss_curl.escape(Api::ss_softname);
    std::string url = "https://www.screenscraper.fr/api2/ssuserInfos.php?devid="
                      + Api::ss_devid + "&devpassword=" + Api::ss_devpassword
                      + "&softname=" + soft + "&output=xml";

    url += ssid.empty() ? "" : "&ssid=" + ssid;
    url += sspassword.empty() ? "" : "&sspassword=" + sspassword;

    SS_PRINT("User: %s\n", url.c_str());

    std::string xml = ss_curl.getString(url, SS_TIMEOUT, &code);
    if (retryDelay > 0) {
        while (code == 429 || code == 28) {
            Api::printError((int) code, retryDelay);
            Io::delay(retryDelay);
            xml = ss_curl.getString(url, SS_TIMEOUT, &code);
        }
    }

    if (code != 0 || xml.empty()) {
        SS_PRINT("User: error %li\n", code);
        http_error = (int) code;
        return;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError e = doc.Parse(xml.c_str(), xml.size());
    if (e != tinyxml2::XML_SUCCESS) {
        SS_PRINT("User: %s\n", doc.ErrorName());
        doc.Clear();
        return;
    }

    tinyxml2::XMLNode *pRoot = doc.FirstChildElement("Data");
    if (pRoot == nullptr) {
        SS_PRINT("User: wrong xml format: \'Data\' tag not found\n");
        doc.Clear();
        return;
    }

    tinyxml2::XMLNode *userNode = pRoot->FirstChildElement("ssuser");
    if (userNode == nullptr) {
        SS_PRINT("User: wrong xml format: \'ssuser\' tag not found\n");
    } else {
        parseUser(this, userNode);
    }

    doc.Clear();
}

bool User::parseUser(User *user, tinyxml2::XMLNode *userNode) {

    if (user == nullptr || userNode == nullptr) {
        return false;
    }

    user->id = Api::getXmlText(userNode->FirstChildElement("id"));
    user->niveau = Api::getXmlText(userNode->FirstChildElement("niveau"));
    user->contribution = Api::getXmlText(userNode->FirstChildElement("contribution"));
    user->uploadsysteme = Api::getXmlText(userNode->FirstChildElement("uploadsysteme"));
    user->uploadinfos = Api::getXmlText(userNode->FirstChildElement("uploadinfos"));
    user->romasso = Api::getXmlText(userNode->FirstChildElement("romasso"));
    user->uploadmedia = Api::getXmlText(userNode->FirstChildElement("uploadmedia"));
    user->maxthreads = Api::getXmlText(userNode->FirstChildElement("maxthreads"));
    user->maxdownloadspeed = Api::getXmlText(userNode->FirstChildElement("maxdownloadspeed"));
    user->requeststoday = Api::getXmlText(userNode->FirstChildElement("requeststoday"));
    user->maxrequestsperday = Api::getXmlText(userNode->FirstChildElement("maxrequestsperday"));
    user->visites = Api::getXmlText(userNode->FirstChildElement("visites"));
    user->datedernierevisite = Api::getXmlText(userNode->FirstChildElement("datedernierevisite"));
    user->favregion = Api::getXmlText(userNode->FirstChildElement("favregion"));

    return true;
}

int User::getMaxThreads() {
    return Api::parseInt(maxthreads, 1);
}
