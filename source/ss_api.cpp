//
// Created by cpasjuste on 05/04/19.
//

#include <algorithm>
#include "ss_io.h"
#include "ss_api.h"
#include "ss_gamelist.h"

using namespace ss_api;

std::string Api::ss_devid;
std::string Api::ss_devpassword;
std::string Api::ss_softname;
bool ss_debug = false;

std::vector<Api::MediaType> Api::mediaTypes(const std::string &ssid, const std::string &sspassword) {

    std::vector<Api::MediaType> mediaTypes;
    long code = 0;
    Curl ss_curl;
    std::string soft = ss_curl.escape(ss_softname);
    std::string url = "https://www.screenscraper.fr/api2/mediasJeuListe.php?devid="
                      + ss_devid + "&devpassword=" + ss_devpassword
                      + "&softname=" + soft + "&output=xml";

    url += ssid.empty() ? "" : "&ssid=" + ssid;
    url += sspassword.empty() ? "" : "&sspassword=" + sspassword;

    SS_PRINT("Api::mediaTypes: %s\n", url.c_str());

    std::string xml = ss_curl.getString(url, SS_TIMEOUT, &code);
    if (xml.empty()) {
        SS_PRINT("Api::mediaTypes: error %li\n", code);
        return mediaTypes;
    }

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError e = doc.Parse(xml.c_str(), xml.size());
    if (e != tinyxml2::XML_SUCCESS) {
        SS_PRINT("Api::mediaTypes: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        doc.Clear();
        return mediaTypes;
    }

    tinyxml2::XMLNode *pRoot = doc.FirstChildElement("Data");
    if (pRoot == nullptr) {
        SS_PRINT("Api::mediaTypes: wrong xml format: \'Data\' tag not found\n");
        doc.Clear();
        return mediaTypes;
    }

    tinyxml2::XMLNode *medias = pRoot->FirstChildElement("medias");
    if (medias == nullptr) {
        SS_PRINT("Api::mediaTypes: wrong xml format: \'medias\' tag not found\n");
    } else {
        tinyxml2::XMLNode *media = medias->FirstChildElement("media");
        while (media != nullptr) {
            mediaTypes.emplace_back(getXmlText(media->FirstChildElement("id")),
                                    getXmlText(media->FirstChildElement("nomcourt")));
            media = media->NextSibling();
        }
    }

    doc.Clear();

    return mediaTypes;
}

Api::GameSearch Api::gameSearch(const std::string &recherche, const std::string &systemeid,
                                const std::string &ssid, const std::string &sspassword) {

    long code = 0;
    Curl ss_curl;
    std::string search = ss_curl.escape(recherche);
    std::string soft = ss_curl.escape(ss_softname);
    std::string url = "https://www.screenscraper.fr/api2/jeuRecherche.php?devid="
                      + ss_devid + "&devpassword=" + ss_devpassword
                      + "&softname=" + soft + "&output=xml" + "&recherche=" + search;

    url += ssid.empty() ? "" : "&ssid=" + ssid;
    url += sspassword.empty() ? "" : "&sspassword=" + sspassword;
    url += systemeid.empty() ? "" : "&systemeid=" + systemeid;

    SS_PRINT("Api::jeuRecherche: %s\n", url.c_str());

    std::string xml = ss_curl.getString(url, SS_TIMEOUT, &code);
    if (xml.empty()) {
        SS_PRINT("Api::jeuRecherche: error %li\n", code);
        return GameSearch((int) code);
    }

    return parseGameSearch(xml);
}

Api::GameSearch Api::parseGameSearch(const std::string &xmlData) {

    GameSearch gs{};
    tinyxml2::XMLDocument doc;

    gs.xml = xmlData;
    tinyxml2::XMLError e = doc.Parse(gs.xml.c_str(), gs.xml.size());
    if (e != tinyxml2::XML_SUCCESS) {
        SS_PRINT("Api::parseGameSearch: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        doc.Clear();
        return gs;
    }

    tinyxml2::XMLNode *pRoot = doc.FirstChildElement("Data");
    if (pRoot == nullptr) {
        SS_PRINT("Api::parseGameSearch: wrong xml format: \'Data\' tag not found\n");
        doc.Clear();
        return gs;
    }

    tinyxml2::XMLNode *userNode = pRoot->FirstChildElement("ssuser");
    if (userNode == nullptr) {
        SS_PRINT("Api::parseGameSearch: wrong xml format: \'ssuser\' tag not found\n");
    } else {
        gs.ssuser = parseUser(userNode);
    }

    tinyxml2::XMLNode *gamesNode = pRoot->FirstChildElement("jeux");
    if (gamesNode == nullptr) {
        SS_PRINT("Api::parseGameSearch: wrong xml format: \'jeux\' tag not found\n");
    } else {
        tinyxml2::XMLNode *gameNode = gamesNode->FirstChildElement("jeu");
        while (gameNode != nullptr) {
            // add game to game list
            gs.games.emplace_back(parseGame(gameNode, ""));
            // move to next node (game)
            gameNode = gameNode->NextSibling();
        }
    }

    doc.Clear();

    return gs;
}

Api::GameInfo
Api::gameInfo(const std::string &crc, const std::string &md5, const std::string &sha1, const std::string &systemeid,
              const std::string &romtype, const std::string &romnom, const std::string &romtaille,
              const std::string &gameid, const std::string &ssid, const std::string &sspassword) {

    long code = 0;
    Curl ss_curl;
    std::string search = ss_curl.escape(romnom);
    std::string soft = ss_curl.escape(ss_softname);
    std::string url = "https://www.screenscraper.fr/api2/jeuInfos.php?devid="
                      + ss_devid + "&devpassword=" + ss_devpassword + "&softname=" + soft + "&output=xml"
                      + "&romnom=" + search;

    url += ssid.empty() ? "" : "&ssid=" + ssid;
    url += sspassword.empty() ? "" : "&sspassword=" + sspassword;
    url += crc.empty() ? "" : "&crc=" + crc;
    url += md5.empty() ? "" : "&md5=" + md5;
    url += sha1.empty() ? "" : "&sha1=" + sha1;
    url += systemeid.empty() ? "" : "&systemeid=" + systemeid;
    url += romtype.empty() ? "" : "&romtype=" + romtype;
    url += romtaille.empty() ? "" : "&romtaille=" + romtaille;
    url += gameid.empty() ? "" : "&gameid=" + gameid;

    SS_PRINT("Api::jeuInfos: %s\n", url.c_str());

    std::string xml = ss_curl.getString(url, SS_TIMEOUT, &code);
    if (xml.empty()) {
        SS_PRINT("Api::jeuInfos: error %li\n", code);
        return GameInfo((int) code);
    }

    return parseGameInfo(xml, romnom);
}

Api::GameInfo Api::parseGameInfo(const std::string &xmlData, const std::string &romName) {

    GameInfo ji{};
    tinyxml2::XMLDocument doc;

    ji.xml = xmlData;
    tinyxml2::XMLError e = doc.Parse(ji.xml.c_str(), ji.xml.size());
    if (e != tinyxml2::XML_SUCCESS) {
        SS_PRINT("Api::parseGameInfo: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        doc.Clear();
        return ji;
    }

    tinyxml2::XMLNode *pRoot = doc.FirstChildElement("Data");
    if (pRoot == nullptr) {
        SS_PRINT("Api::parseGameInfo: wrong xml format: \'Data\' tag not found\n");
        doc.Clear();
        return ji;
    }

    tinyxml2::XMLNode *userNode = pRoot->FirstChildElement("ssuser");
    if (userNode == nullptr) {
        SS_PRINT("Api::parseGameInfo: wrong xml format: \'ssuser\' tag not found\n");
    } else {
        ji.ssuser = parseUser(userNode);
    }

    tinyxml2::XMLNode *gameNode = pRoot->FirstChildElement("jeu");
    if (gameNode == nullptr) {
        SS_PRINT("Api::parseGameInfo: wrong xml format: \'jeu\' tag not found\n");
    } else {
        ji.game = parseGame(gameNode, romName);
    }

    doc.Clear();

    return ji;
}

Game Api::parseGame(tinyxml2::XMLNode *gameNode, const std::string &romName, const GameList::Format &format) {

    Game game{};
    tinyxml2::XMLElement *element = nullptr;

    if (gameNode == nullptr) {
        return game;
    }

    if (format == GameList::Format::FbNeo) {
        game.names.emplace_back("wor", Api::getXmlText(gameNode->FirstChildElement("description")));
        game.path = Api::getXmlAttribute(gameNode->ToElement(), "name");
        if (!game.path.empty()) {
            game.path += ".zip";
        }
        game.dates.emplace_back("wor", getXmlText(gameNode->FirstChildElement("year")));
        game.developer.text = getXmlText(gameNode->FirstChildElement("manufacturer"));
        game.editor.text = game.developer.text;
        game.cloneof = Api::getXmlAttribute(gameNode->ToElement(), "cloneof");
        return game;
    }

    // screenscraper / emulationstation compat
    game.id = getXmlAttribute(gameNode->ToElement(), "id");
    // screenscraper
    game.romid = getXmlAttribute(gameNode->ToElement(), "romid");
    // screenscraper
    game.notgame = getXmlAttribute(gameNode->ToElement(), "notgame");
    // emulationstation compat
    game.source = getXmlAttribute(gameNode->ToElement(), "source");
    // emulationstation compat
    game.path = getXmlText(gameNode->FirstChildElement("path"));
    if (game.path.empty() || game.path == "Unknown") {
        game.path = romName;
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("noms");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("nom");
        while (node != nullptr) {
            game.names.emplace_back(getXmlAttribute(node->ToElement(), "region"),
                                    getXmlText(node->ToElement()));
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game.names.emplace_back(Api::toString(Game::Country::WOR),
                                getXmlText(gameNode->FirstChildElement("name")));
    }
    // screenscraper
    element = gameNode->FirstChildElement("regions");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("region");
        while (node != nullptr) {
            game.countries.emplace_back(getXmlText(node->ToElement()));
            node = node->NextSibling();
        }
    }
    // screenscraper
    game.cloneof = getXmlText(gameNode->FirstChildElement("cloneof"));
    // screenscraper
    game.system.id = getXmlAttribute(gameNode->FirstChildElement("systeme"), "id");
    game.system.text = getXmlText(gameNode->FirstChildElement("systeme"));
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("synopsis");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("synopsis");
        while (node != nullptr) {
            Game::Synopsis synopsis{};
            synopsis.language = getXmlAttribute(node->ToElement(), "langue");
            synopsis.text = getXmlText(node->ToElement());
            game.synopses.emplace_back(synopsis);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game.synopses.emplace_back("wor", getXmlText(gameNode->FirstChildElement("desc")));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("medias");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("media");
        while (node != nullptr) {
            Game::Media media{};
            media.parent = getXmlAttribute(node->ToElement(), "parent");
            media.type = getXmlAttribute(node->ToElement(), "type");
            media.crc = getXmlAttribute(node->ToElement(), "crc");
            media.md5 = getXmlAttribute(node->ToElement(), "md5");
            media.sha1 = getXmlAttribute(node->ToElement(), "sha1");
            media.format = getXmlAttribute(node->ToElement(), "format");
            media.support = getXmlAttribute(node->ToElement(), "support");
            media.url = getXmlText(node->ToElement());
            media.country = getXmlAttribute(node->ToElement(), "region");
            if (media.country.empty()) {
                media.country = toString(Game::Country::UNKNOWN);
            }
            game.medias.emplace_back(media);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat
        game.medias.push_back({"sstitle", "",
                               getXmlText(gameNode->FirstChildElement("image")), "wor", "", "", "", "", ""});
        game.medias.push_back({"ss", "",
                               getXmlText(gameNode->FirstChildElement("thumbnail")), "wor", "", "", "", "", ""});
        game.medias.push_back({"video", "",
                               getXmlText(gameNode->FirstChildElement("video")), "wor", "", "", "", "", ""});
    }
    // screenscraper (prioritise screenscraper format)
    game.rating = getXmlText(gameNode->FirstChildElement("note"));
    if (game.rating.empty()) {
        // emulationstation compat (use emulationstation format)
        game.rating = getXmlText(gameNode->FirstChildElement("rating"));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("dates");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("date");
        while (node != nullptr) {
            Game::Date date{};
            date.country = getXmlAttribute(node->ToElement(), "region");
            date.text = getXmlText(node->ToElement());
            if (date.text.size() >= 4) {
                date.text = date.text.substr(0, 4);
            }
            game.dates.emplace_back(date);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game.dates.emplace_back("wor", getXmlText(gameNode->FirstChildElement("releasedate")));
    }
    // screenscraper
    game.developer.id = getXmlAttribute(gameNode->FirstChildElement("developpeur"), "id");
    game.developer.text = getXmlText(gameNode->FirstChildElement("developpeur"));
    if (game.developer.text.empty()) {
        // emulationstation compat (use emulationstation format)
        game.developer.text = getXmlText(gameNode->FirstChildElement("developer"));
    }
    // screenscraper
    game.editor.id = getXmlAttribute(gameNode->FirstChildElement("editeur"), "id");
    game.editor.text = getXmlText(gameNode->FirstChildElement("editeur"));
    if (game.editor.text.empty()) {
        // emulationstation compat (use emulationstation format)
        game.editor.text = getXmlText(gameNode->FirstChildElement("publisher"));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("genres");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("genre");
        while (node != nullptr) {
            Game::Genre genre{};
            genre.id = getXmlAttribute(node->ToElement(), "id");
            genre.main = getXmlAttribute(node->ToElement(), "principale");
            genre.parentid = getXmlAttribute(node->ToElement(), "parentid");
            genre.language = getXmlAttribute(node->ToElement(), "langue");
            genre.text = getXmlText(node->ToElement());
            game.genres.emplace_back(genre);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game.genres.emplace_back(
                "", "", "", "en", getXmlText(gameNode->FirstChildElement("genre")));
    }
    // screenscraper
    game.players = getXmlText(gameNode->FirstChildElement("joueurs"));
    if (game.players.empty()) {
        // emulationstation compat (use emulationstation format)
        game.players = getXmlText(gameNode->FirstChildElement("players"));
    }
    // screenscraper
    game.topstaff = getXmlText(gameNode->FirstChildElement("topstaff"));
    // screenscraper
    game.rotation = getXmlText(gameNode->FirstChildElement("rotation"));
    // screenscraper
    game.resolution = getXmlText(gameNode->FirstChildElement("resolution"));
    // screenscraper
    game.inputs = getXmlText(gameNode->FirstChildElement("controles"));
    // screenscraper
    game.colors = getXmlText(gameNode->FirstChildElement("couleurs"));

    return game;
}

User Api::parseUser(tinyxml2::XMLNode *userNode) {

    User user{};

    if (userNode == nullptr) {
        return user;
    }

    user.id = getXmlText(userNode->FirstChildElement("id"));
    user.niveau = getXmlText(userNode->FirstChildElement("niveau"));
    user.contribution = getXmlText(userNode->FirstChildElement("contribution"));
    user.uploadsysteme = getXmlText(userNode->FirstChildElement("uploadsysteme"));
    user.uploadinfos = getXmlText(userNode->FirstChildElement("uploadinfos"));
    user.romasso = getXmlText(userNode->FirstChildElement("romasso"));
    user.uploadmedia = getXmlText(userNode->FirstChildElement("uploadmedia"));
    user.maxthreads = getXmlText(userNode->FirstChildElement("maxthreads"));
    user.maxdownloadspeed = getXmlText(userNode->FirstChildElement("maxdownloadspeed"));
    user.requeststoday = getXmlText(userNode->FirstChildElement("requeststoday"));
    user.maxrequestsperday = getXmlText(userNode->FirstChildElement("maxrequestsperday"));
    user.visites = getXmlText(userNode->FirstChildElement("visites"));
    user.datedernierevisite = getXmlText(userNode->FirstChildElement("datedernierevisite"));
    user.favregion = getXmlText(userNode->FirstChildElement("favregion"));

    return user;
}

std::string Api::getXmlAttribute(tinyxml2::XMLElement *element, const std::string &name) {

    if (element == nullptr || element->Attribute(name.c_str()) == nullptr) {
        return "";
    }

    return element->Attribute(name.c_str());
}

std::string Api::getXmlText(tinyxml2::XMLElement *element) {

    if (element == nullptr || element->GetText() == nullptr) {
        return "";
    }

    return element->GetText();
}

tinyxml2::XMLElement *Api::addXmlElement(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *parent,
                                         const std::string &name, const std::string &value) {

    if (parent == nullptr) {
        return nullptr;
    }

    if (!value.empty()) {
        tinyxml2::XMLElement *element = doc->NewElement(name.c_str());
        element->SetText(value.c_str());
        parent->InsertEndChild(element);
        return element;
    }

    return nullptr;
}

std::string Api::toString(const Game::Language &language) {
    switch (language) {
        case Game::Language::EN:
            return "en";
        case Game::Language::FR:
            return "fr";
        case Game::Language::ES:
            return "es";
        case Game::Language::PT:
            return "pt";
        default:
            return "unknown";
    }
}

std::string Api::toString(const Game::Country &region) {
    switch (region) {
        case Game::Country::DE:
            return "de";
        case Game::Country::ASI:
            return "asi";
        case Game::Country::AU:
            return "au";
        case Game::Country::BR:
            return "br";
        case Game::Country::BG:
            return "bg";
        case Game::Country::CA:
            return "ca";
        case Game::Country::CL:
            return "cl";
        case Game::Country::CN:
            return "cn";
        case Game::Country::AME:
            return "ame";
        case Game::Country::KR:
            return "kr";
        case Game::Country::CUS:
            return "cus";
        case Game::Country::DK:
            return "dk";
        case Game::Country::SP:
            return "sp";
        case Game::Country::EU:
            return "eu";
        case Game::Country::FI:
            return "fi";
        case Game::Country::FR:
            return "fr";
        case Game::Country::GR:
            return "gr";
        case Game::Country::HU:
            return "hu";
        case Game::Country::IL:
            return "il";
        case Game::Country::IT:
            return "it";
        case Game::Country::JP:
            return "jp";
        case Game::Country::KW:
            return "kw";
        case Game::Country::WOR:
            return "wor";
        case Game::Country::MOR:
            return "mor";
        case Game::Country::NO:
            return "no";
        case Game::Country::NZ:
            return "nz";
        case Game::Country::OCE:
            return "oce";
        case Game::Country::NL:
            return "nl";
        case Game::Country::PE:
            return "pe";
        case Game::Country::PL:
            return "pl";
        case Game::Country::PT:
            return "pt";
        case Game::Country::CZ:
            return "cz";
        case Game::Country::UK:
            return "uk";
        case Game::Country::RU:
            return "ru";
        case Game::Country::SS:
            return "ss";
        case Game::Country::SK:
            return "sk";
        case Game::Country::SE:
            return "se";
        case Game::Country::TW:
            return "tw";
        case Game::Country::TR:
            return "tr";
        case Game::Country::US:
            return "us";
        case Game::Country::ALL:
            return "all";
        default:
            return "Unknown";
    }
}

Game::Country Api::toCountry(const std::string &country) {
    if (country == "de") { return Game::Country::US; }
    else if (country == "asi") { return Game::Country::ASI; }
    else if (country == "au") { return Game::Country::AU; }
    else if (country == "br") { return Game::Country::BR; }
    else if (country == "bg") { return Game::Country::BG; }
    else if (country == "ca") { return Game::Country::CA; }
    else if (country == "cl") { return Game::Country::CL; }
    else if (country == "cn") { return Game::Country::CN; }
    else if (country == "ame") { return Game::Country::AME; }
    else if (country == "kr") { return Game::Country::KR; }
    else if (country == "cus") { return Game::Country::CUS; }
    else if (country == "dk") { return Game::Country::DK; }
    else if (country == "sp") { return Game::Country::SP; }
    else if (country == "eu") { return Game::Country::EU; }
    else if (country == "fi") { return Game::Country::FI; }
    else if (country == "fr") { return Game::Country::FR; }
    else if (country == "gr") { return Game::Country::GR; }
    else if (country == "hu") { return Game::Country::HU; }
    else if (country == "il") { return Game::Country::IL; }
    else if (country == "it") { return Game::Country::IL; }
    else if (country == "jp") { return Game::Country::JP; }
    else if (country == "kw") { return Game::Country::KW; }
    else if (country == "wor") { return Game::Country::WOR; }
    else if (country == "mor") { return Game::Country::MOR; }
    else if (country == "no") { return Game::Country::NO; }
    else if (country == "nz") { return Game::Country::NZ; }
    else if (country == "oce") { return Game::Country::OCE; }
    else if (country == "nl") { return Game::Country::NL; }
    else if (country == "pe") { return Game::Country::PE; }
    else if (country == "pl") { return Game::Country::PL; }
    else if (country == "pt") { return Game::Country::PT; }
    else if (country == "cz") { return Game::Country::CZ; }
    else if (country == "uk") { return Game::Country::UK; }
    else if (country == "ru") { return Game::Country::RU; }
    else if (country == "ss") { return Game::Country::SS; }
    else if (country == "sk") { return Game::Country::SK; }
    else if (country == "se") { return Game::Country::SE; }
    else if (country == "tw") { return Game::Country::TW; }
    else if (country == "tr") { return Game::Country::TR; }
    else if (country == "us") { return Game::Country::US; }
    else if (country == "all") { return Game::Country::ALL; }
    else return Game::Country::UNKNOWN;
}

Game::Language Api::toLanguage(const std::string &language) {
    if (language == "en") { return Game::Language::EN; }
    else if (language == "fr") { return Game::Language::FR; }
    else if (language == "es") { return Game::Language::ES; }
    else if (language == "pt") { return Game::Language::PT; }
    else if (language == "all") { return Game::Language::ALL; }
    else return Game::Language::UNKNOWN;
}

bool Api::sortByName(const std::string &g1, const std::string &g2) {
    return strcasecmp(g1.c_str(), g2.c_str()) <= 0;
}

bool Api::sortGameByName(const Game &g1, const Game &g2) {
    return g1.getName().text < g2.getName().text;
}

