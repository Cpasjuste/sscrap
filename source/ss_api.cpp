//
// Created by cpasjuste on 05/04/19.
//

#include <cstring>
#include "ss_api.h"

using namespace ss_api;
using namespace tinyxml2;

std::string Api::ss_devid;
std::string Api::ss_devpassword;
std::string Api::ss_softname;

Api::GameSearch Api::gameSearch(const std::string &recherche, const std::string &systemeid,
                                const std::string &ssid, const std::string &sspassword) {

    long code = 0;
    Curl ss_curl;
    std::string search = ss_curl.escape(recherche);
    std::string soft = ss_curl.escape(ss_softname);
    std::string url = "https://www.screenscraper.fr/api2/jeuRecherche.php?devid="
                      + ss_devid + "&devpassword=" + ss_devpassword + "&softname=" + soft + "&output=xml"
                      + "&recherche=" + search;
    if (!ssid.empty()) {
        url += "&ssid=" + ssid;
    }
    if (!sspassword.empty()) {
        url += "&sspassword=" + sspassword;
    }
    if (!systemeid.empty()) {
        url += "&systemeid=" + systemeid;
    }

    printf("Api::jeuRecherche: %s\n", url.c_str());

    std::string xml = ss_curl.getString(url, SS_TIMEOUT, &code);
    if (xml.empty()) {
        printf("Api::jeuRecherche: error %li\n", code);
        return GameSearch();
    }

    return parseGameSearch(xml);
}

Api::GameSearch Api::gameSearch(const std::string &srcPath) {

    long size = 0;
    FILE *fp = fopen(srcPath.c_str(), "rb");
    if (!fp) {
        printf("Api::jeuRecherche: error: fopen failed\n");
        return GameSearch();
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size <= 0) {
        printf("Api::jeuRecherche: error: ftell failed\n");
        fclose(fp);
        return GameSearch();
    }
    fseek(fp, 0, SEEK_SET);
    std::string xml = std::string((unsigned long) size, '\0');
    fread(&xml[0], sizeof(char), (size_t) size, fp);
    fclose(fp);

    if (xml.empty()) {
        printf("Api::jeuRecherche: error: data is empty\n");
        return GameSearch();
    }

    return parseGameSearch(xml);
}

Api::GameSearch Api::parseGameSearch(const std::string &xmlData) {

    GameSearch gs{};
    XMLDocument doc;

    gs.xml = xmlData;
    XMLError e = doc.Parse(gs.xml.c_str(), gs.xml.size());
    if (e != XML_SUCCESS) {
        printf("Api::parseGameSearch: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return gs;
    }

    XMLNode *pRoot = doc.FirstChildElement("Data");
    if (!pRoot) {
        printf("Api::parseGameSearch: wrong xml format: \'Data\' tag not found\n");
        return gs;
    }

    XMLNode *userNode = pRoot->FirstChildElement("ssuser");
    if (!userNode) {
        printf("Api::parseGameSearch: wrong xml format: \'ssuser\' tag not found\n");
    } else {
        gs.ssuser = parseUser(userNode);
    }

    XMLNode *gamesNode = pRoot->FirstChildElement("jeux");
    if (!gamesNode) {
        printf("Api::parseGameSearch: wrong xml format: \'jeux\' tag not found\n");
    } else {
        XMLNode *gameNode = gamesNode->FirstChildElement("jeu");
        while (gameNode) {
            // add game to game list
            gs.games.emplace_back(parseGame(gameNode));
            // move to next node (game)
            gameNode = gameNode->NextSibling();
        }
    }

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
    if (!ssid.empty()) {
        url += "&ssid=" + ssid;
    }
    if (!sspassword.empty()) {
        url += "&sspassword=" + sspassword;
    }
    if (!systemeid.empty()) {
        url += "&systemeid=" + systemeid;
    }
    if (!crc.empty()) {
        url += "&crc=" + crc;
    }
    if (!md5.empty()) {
        url += "&md5=" + md5;
    }
    if (!sha1.empty()) {
        url += "&sha1=" + sha1;
    }
    if (!systemeid.empty()) {
        url += "&systemeid=" + systemeid;
    }
    if (!romtype.empty()) {
        url += "&romtype=" + romtype;
    }
    if (!romtaille.empty()) {
        url += "&romtaille=" + romtaille;
    }
    if (!gameid.empty()) {
        url += "&gameid=" + gameid;
    }

    printf("Api::jeuInfos: %s\n", url.c_str());

    std::string xml = ss_curl.getString(url, SS_TIMEOUT, &code);
    if (xml.empty()) {
        printf("Api::jeuInfos: error %li\n", code);
        return GameInfo();
    }

    return parseGameInfo(xml);
}

Api::GameInfo Api::gameInfo(const std::string &srcPath) {

    long size = 0;
    FILE *fp = fopen(srcPath.c_str(), "rb");
    if (!fp) {
        printf("Api::jeuInfos: error: fopen failed: %s\n", srcPath.c_str());
        return GameInfo();
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size <= 0) {
        printf("Api::jeuInfos: error: ftell failed\n");
        fclose(fp);
        return GameInfo();
    }
    fseek(fp, 0, SEEK_SET);
    std::string xml = std::string((unsigned long) size, '\0');
    fread(&xml[0], sizeof(char), (size_t) size, fp);
    fclose(fp);

    if (xml.empty()) {
        printf("Api::jeuInfos: error: data is empty\n");
        return GameInfo();
    }

    return parseGameInfo(xml);
}

Api::GameInfo Api::parseGameInfo(const std::string &xmlData) {

    GameInfo ji{};
    XMLDocument doc;

    ji.xml = xmlData;
    XMLError e = doc.Parse(ji.xml.c_str(), ji.xml.size());
    if (e != XML_SUCCESS) {
        printf("Api::parseGameInfo: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return ji;
    }

    XMLNode *pRoot = doc.FirstChildElement("Data");
    if (!pRoot) {
        printf("Api::parseGameInfo: wrong xml format: \'Data\' tag not found\n");
        return ji;
    }

    XMLNode *userNode = pRoot->FirstChildElement("ssuser");
    if (!userNode) {
        printf("Api::parseGameInfo: wrong xml format: \'ssuser\' tag not found\n");
    } else {
        ji.ssuser = parseUser(userNode);
    }

    XMLNode *gameNode = pRoot->FirstChildElement("jeu");
    if (!gameNode) {
        printf("Api::parseGameInfo: wrong xml format: \'jeu\' tag not found\n");
    } else {
        ji.game = parseGame(gameNode);
    }

    return ji;
}

Api::GameList Api::gameList(const std::string &xmlPath) {

    GameList gl{};
    XMLDocument doc;

    gl.xml = xmlPath;
    XMLError e = doc.LoadFile(xmlPath.c_str());
    if (e != XML_SUCCESS) {
        printf("Api::gameList: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return gl;
    }

    XMLNode *pRoot = doc.FirstChildElement("Data");
    if (!pRoot) {
        // emulationstation format
        pRoot = doc.FirstChildElement("gameList");
        if (!pRoot) {
            printf("Api::parseGameSearch: wrong xml format: \'Data\' or \'gameList\' tag not found\n");
            return gl;
        }
    }

    XMLNode *gamesNode = pRoot->FirstChildElement("jeux");
    if (!gamesNode) {
        // emulationstation format
        gamesNode = pRoot;
    }

    XMLNode *gameNode = gamesNode->FirstChildElement("jeu");
    if (!gameNode) {
        gameNode = gamesNode->FirstChildElement("game");
    }
    while (gameNode) {
        // add game to game list
        gl.games.emplace_back(parseGame(gameNode));
        // move to next node (game)
        gameNode = gameNode->NextSibling();
    }

    return gl;
}

Game Api::parseGame(XMLNode *gameNode) {

    Game game{};

    if (!gameNode) {
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
    // screenscraper (prioritise screenscraper format)
    XMLElement *element = gameNode->FirstChildElement("noms");
    if (element) {
        XMLNode *node = element->FirstChildElement("nom");
        while (node) {
            Game::Name gameName{};
            gameName.country = getXmlAttribute(node->ToElement(), "region");
            gameName.text = getXmlText(node->ToElement());
            game.names.emplace_back(gameName);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game.names.push_back({"wor", getXmlText(gameNode->FirstChildElement("name"))});
    }
    // screenscraper
    element = gameNode->FirstChildElement("regions");
    if (element) {
        XMLNode *node = element->FirstChildElement("region");
        while (node) {
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
    if (element) {
        XMLNode *node = element->FirstChildElement("synopsis");
        while (node) {
            Game::Synopsis synopsis{};
            synopsis.language = getXmlAttribute(node->ToElement(), "langue");
            synopsis.text = getXmlText(node->ToElement());
            game.synopses.emplace_back(synopsis);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game.synopses.push_back({"wor", getXmlText(gameNode->FirstChildElement("desc"))});
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("medias");
    if (element) {
        XMLNode *node = element->FirstChildElement("media");
        while (node) {
            Game::Media media{};
            media.parent = getXmlAttribute(node->ToElement(), "parent");
            media.type = getXmlAttribute(node->ToElement(), "type");
            media.country = getXmlAttribute(node->ToElement(), "region");
            media.crc = getXmlAttribute(node->ToElement(), "crc");
            media.md5 = getXmlAttribute(node->ToElement(), "md5");
            media.sha1 = getXmlAttribute(node->ToElement(), "sha1");
            media.format = getXmlAttribute(node->ToElement(), "format");
            media.support = getXmlAttribute(node->ToElement(), "support");
            media.url = getXmlText(node->ToElement());
            game.medias.emplace_back(media);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat
        game.medias.push_back({"sstitle", "",
                               getXmlText(gameNode->FirstChildElement("image")), "wor", "", "", "", "", ""});
        game.medias.push_back({"screenshot", "",
                               getXmlText(gameNode->FirstChildElement("thumbnail")), "wor", "", "", "", "", ""});
    }
    // screenscraper (prioritise screenscraper format)
    game.rating = getXmlText(gameNode->FirstChildElement("note"));
    if (game.rating.empty()) {
        // emulationstation compat (use emulationstation format)
        game.rating = getXmlText(gameNode->FirstChildElement("rating"));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("dates");
    if (element) {
        XMLNode *node = element->FirstChildElement("date");
        while (node) {
            Game::Date date{};
            date.country = getXmlAttribute(node->ToElement(), "region");
            date.text = getXmlText(node->ToElement());
            game.dates.emplace_back(date);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        Game::Date date{"wor", getXmlText(gameNode->FirstChildElement("releasedate"))};
        game.dates.emplace_back(date);
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
    if (element) {
        XMLNode *node = element->FirstChildElement("genre");
        while (node) {
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
        Game::Genre genre{"", "", "", "en", getXmlText(gameNode->FirstChildElement("genre"))};
        game.genres.emplace_back(genre);
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

User Api::parseUser(XMLNode *userNode) {

    User user{};

    if (!userNode) {
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

int Api::download(const Game::Media &media, const std::string &dstPath) {

    if (dstPath.empty()) {
        return -1;
    }

    printf("Api::download: %s\n", media.url.c_str());

    long http_code = 0;
    Curl ss_curl;
    int res = ss_curl.getData(media.url, dstPath, SS_TIMEOUT, &http_code);
    if (res != 0) {
        printf("Api::download: error: curl failed: %s, http_code: %li\n",
               curl_easy_strerror((CURLcode) res), http_code);
        return (int) http_code;
    }

    return 0;
}

bool Api::GameInfo::save(const std::string &dstPath) {

    FILE *fp = fopen(dstPath.c_str(), "wb");
    if (!fp) {
        printf("Api::GameInfo::save: error: fopen failed\n");
        return false;
    }

    fwrite(xml.c_str(), sizeof(char), xml.length(), fp);
    fclose(fp);

    return true;
}

bool Api::GameSearch::save(const std::string &dstPath) {
    FILE *fp = fopen(dstPath.c_str(), "wb");
    if (!fp) {
        printf("Api::GameSearch::save: error: fopen failed\n");
        return false;
    }

    fwrite(xml.c_str(), sizeof(char), xml.length(), fp);
    fclose(fp);

    return true;
}

bool Api::GameList::save(const std::string &dstPath) {
    FILE *fp = fopen(dstPath.c_str(), "wb");
    if (!fp) {
        printf("Api::GameList::save: error: fopen failed\n");
        return -1;
    }

    fwrite(xml.c_str(), sizeof(char), xml.length(), fp);
    fclose(fp);

    return true;
}

std::string Api::getXmlAttribute(tinyxml2::XMLElement *element, const std::string &name) {

    if (!element || !element->Attribute(name.c_str())) {
        return "";
    }

    return element->Attribute(name.c_str());
}

std::string Api::getXmlText(tinyxml2::XMLElement *element) {

    if (!element || !element->GetText()) {
        return "";
    }

    return element->GetText();
}

std::string Api::toString(const Game::Media::Type &type) {
    switch (type) {
        case Game::Media::Type::SSTitle:
            return "sstitle";
        case Game::Media::Type::SS:
            return "ss";
        case Game::Media::Type::Screenshot:
            return "screenshot";
        case Game::Media::Type::Fanart:
            return "fanart";
        case Game::Media::Type::Video:
            return "video";
        case Game::Media::Type::Marquee:
            return "marquee";
        case Game::Media::Type::ScreenMarquee:
            return "screenmarquee";
        case Game::Media::Type::ScreenMarqueeSmall:
            return "screenmarqueesmall";
        case Game::Media::Type::ThemeHs:
            return "themehs";
        case Game::Media::Type::Manuel:
            return "manuel";
        case Game::Media::Type::Flyer:
            return "flyer";
        case Game::Media::Type::SteamGrid:
            return "steamgrid";
        case Game::Media::Type::Wheel:
            return "wheel";
        case Game::Media::Type::WheelHD:
            return "wheel-hd";
        case Game::Media::Type::WheelCarbon:
            return "wheel-carbon";
        case Game::Media::Type::WheelSteel:
            return "wheel-steel";
        case Game::Media::Type::Box2D:
            return "box-2D";
        case Game::Media::Type::Box2DSide:
            return "box-2D-side";
        case Game::Media::Type::Box2DBack:
            return "box-2D-back";
        case Game::Media::Type::BoxTexture:
            return "box-texture";
        case Game::Media::Type::Box3D:
            return "box-3D";
        case Game::Media::Type::BoxScan:
            return "box-scan";
        case Game::Media::Type::SupportTexture:
            return "support-texture";
        case Game::Media::Type::Bezel43:
            return "bezel-4-3";
        case Game::Media::Type::Bezel169:
            return "bezel-16-9";
        case Game::Media::Type::Bezel1610:
            return "bezel-16-10";
        case Game::Media::Type::Mixrbv1:
            return "mixrbv1";
        case Game::Media::Type::Mixrbv2:
            return "mixrbv2";
        case Game::Media::Type::Pictoliste:
            return "pictoliste";
        case Game::Media::Type::Pictocouleur:
            return "pictocouleur";
        case Game::Media::Type::Pictomonochrome:
            return "pictomonochrome";
        default:
            return "unknown";
    }
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
            return "unknown";
    }
}

Game::Media::Type Api::toMedia(const std::string &type) {

    if (type == "sstitle") { return Game::Media::Type::SSTitle; }
    else if (type == "ss") { return Game::Media::Type::SS; }
    else if (type == "screenshot") { return Game::Media::Type::Screenshot; }
    else if (type == "fanart") { return Game::Media::Type::Fanart; }
    else if (type == "video") { return Game::Media::Type::Video; }
    else if (type == "marquee") { return Game::Media::Type::Marquee; }
    else if (type == "screenmarquee") { return Game::Media::Type::ScreenMarquee; }
    else if (type == "screenmarqueesmall") { return Game::Media::Type::ScreenMarqueeSmall; }
    else if (type == "themehs") { return Game::Media::Type::ThemeHs; }
    else if (type == "manuel") { return Game::Media::Type::Manuel; }
    else if (type == "flyer") { return Game::Media::Type::Flyer; }
    else if (type == "steamgrid") { return Game::Media::Type::SteamGrid; }
    else if (type == "wheel") { return Game::Media::Type::Wheel; }
    else if (type == "wheel-hd") { return Game::Media::Type::WheelHD; }
    else if (type == "wheel-carbon") { return Game::Media::Type::WheelCarbon; }
    else if (type == "wheel-steel") { return Game::Media::Type::WheelSteel; }
    else if (type == "box-2D") { return Game::Media::Type::Box2D; }
    else if (type == "box-2D-side") { return Game::Media::Type::Box2DSide; }
    else if (type == "box-2D-back") { return Game::Media::Type::Box2DBack; }
    else if (type == "box-texture") { return Game::Media::Type::BoxTexture; }
    else if (type == "box-2D-side") { return Game::Media::Type::Box2DSide; }
    else if (type == "box-3D") { return Game::Media::Type::Box3D; }
    else if (type == "box-scan") { return Game::Media::Type::BoxScan; }
    else if (type == "support-texture") { return Game::Media::Type::SupportTexture; }
    else if (type == "box-2D-side") { return Game::Media::Type::Box2DSide; }
    else if (type == "bezel-4-3") { return Game::Media::Type::Bezel43; }
    else if (type == "bezel-16-9") { return Game::Media::Type::Bezel169; }
    else if (type == "bezel-16-10") { return Game::Media::Type::Bezel1610; }
    else if (type == "mixrbv1") { return Game::Media::Type::Mixrbv1; }
    else if (type == "mixrbv2") { return Game::Media::Type::Mixrbv2; }
    else if (type == "pictoliste") { return Game::Media::Type::Pictoliste; }
    else if (type == "pictocouleur") { return Game::Media::Type::Pictocouleur; }
    else if (type == "pictomonochrome") { return Game::Media::Type::Pictomonochrome; }
    else { return Game::Media::Type::Unknow; }
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

