//
// Created by cpasjuste on 05/04/19.
//

#include <algorithm>
#include "ss_io.h"
#include "ss_api.h"

using namespace ss_api;
using namespace tinyxml2;

std::string Api::ss_devid;
std::string Api::ss_devpassword;
std::string Api::ss_softname;
bool ss_debug = false;

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
    XMLDocument doc;

    gs.xml = xmlData;
    XMLError e = doc.Parse(gs.xml.c_str(), gs.xml.size());
    if (e != XML_SUCCESS) {
        SS_PRINT("Api::parseGameSearch: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return gs;
    }

    XMLNode *pRoot = doc.FirstChildElement("Data");
    if (!pRoot) {
        SS_PRINT("Api::parseGameSearch: wrong xml format: \'Data\' tag not found\n");
        return gs;
    }

    XMLNode *userNode = pRoot->FirstChildElement("ssuser");
    if (!userNode) {
        SS_PRINT("Api::parseGameSearch: wrong xml format: \'ssuser\' tag not found\n");
    } else {
        gs.ssuser = parseUser(userNode);
    }

    XMLNode *gamesNode = pRoot->FirstChildElement("jeux");
    if (!gamesNode) {
        SS_PRINT("Api::parseGameSearch: wrong xml format: \'jeux\' tag not found\n");
    } else {
        XMLNode *gameNode = gamesNode->FirstChildElement("jeu");
        while (gameNode) {
            // add game to game list
            gs.games.emplace_back(parseGame(gameNode, ""));
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
    XMLDocument doc;

    ji.xml = xmlData;
    XMLError e = doc.Parse(ji.xml.c_str(), ji.xml.size());
    if (e != XML_SUCCESS) {
        SS_PRINT("Api::parseGameInfo: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return ji;
    }

    XMLNode *pRoot = doc.FirstChildElement("Data");
    if (!pRoot) {
        SS_PRINT("Api::parseGameInfo: wrong xml format: \'Data\' tag not found\n");
        return ji;
    }

    XMLNode *userNode = pRoot->FirstChildElement("ssuser");
    if (!userNode) {
        SS_PRINT("Api::parseGameInfo: wrong xml format: \'ssuser\' tag not found\n");
    } else {
        ji.ssuser = parseUser(userNode);
    }

    XMLNode *gameNode = pRoot->FirstChildElement("jeu");
    if (!gameNode) {
        SS_PRINT("Api::parseGameInfo: wrong xml format: \'jeu\' tag not found\n");
    } else {
        ji.game = parseGame(gameNode, romName);
    }

    return ji;
}

Api::GameList Api::gameList(const std::string &xmlPath, const std::string &rPath) {

    GameList gl{};
    XMLDocument doc;
    std::vector<std::string> files;

    gl.xml = xmlPath;
    XMLError e = doc.LoadFile(xmlPath.c_str());
    if (e != XML_SUCCESS) {
        SS_PRINT("Api::gameList: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return gl;
    }

    XMLNode *pRoot = doc.FirstChildElement("Data");
    if (!pRoot) {
        // emulationstation format
        pRoot = doc.FirstChildElement("gameList");
        if (!pRoot) {
            SS_PRINT("Api::parseGameSearch: wrong xml format: \'Data\' or \'gameList\' tag not found\n");
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

    gl.romPath = rPath;
    if (!gl.romPath.empty()) {
        files = Io::getDirList(gl.romPath);
    }

    while (gameNode) {
        Game game = parseGame(gameNode);
        // is rom available?
        auto p = std::find(files.begin(), files.end(), game.path);
        if (p != files.end()) {
            game.available = true;
            gl.roms_count++;
        }
        // add stuff for later filtering
        p = std::find(gl.systems.begin(), gl.systems.end(), game.system.text);
        if (p == gl.systems.end()) {
            gl.systems.emplace_back(game.system.text);
        }
        p = std::find(gl.editors.begin(), gl.editors.end(), game.editor.text);
        if (p == gl.editors.end()) {
            gl.editors.emplace_back(game.editor.text);
        }
        p = std::find(gl.developers.begin(), gl.developers.end(), game.developer.text);
        if (p == gl.developers.end()) {
            gl.developers.emplace_back(game.developer.text);
        }
        p = std::find(gl.players.begin(), gl.players.end(), game.players);
        if (p == gl.players.end()) {
            gl.players.emplace_back(game.players);
        }
        p = std::find(gl.ratings.begin(), gl.ratings.end(), game.rating);
        if (p == gl.ratings.end()) {
            gl.ratings.emplace_back(game.rating);
        }
        p = std::find(gl.topstaffs.begin(), gl.topstaffs.end(), game.topstaff);
        if (p == gl.topstaffs.end()) {
            gl.topstaffs.emplace_back(game.topstaff);
        }
        p = std::find(gl.rotations.begin(), gl.rotations.end(), game.rotation);
        if (p == gl.rotations.end()) {
            gl.rotations.emplace_back(game.rotation);
        }
        p = std::find(gl.resolutions.begin(), gl.resolutions.end(), game.resolution);
        if (p == gl.resolutions.end()) {
            gl.resolutions.emplace_back(game.resolution);
        }
        if (!game.dates.empty()) {
            std::string date = game.getDate(Game::Country::WOR).text;
            p = std::find(gl.dates.begin(), gl.dates.end(), date);
            if (p == gl.dates.end()) {
                gl.dates.emplace_back(date);
            }
        }
        if (!game.genres.empty()) {
            std::string genre = game.getGenre(Game::Language::EN).text;
            p = std::find(gl.genres.begin(), gl.genres.end(), genre);
            if (p == gl.genres.end()) {
                gl.genres.emplace_back(genre);
            }
        }
        // add game to game list
        gl.games.emplace_back(game);
        // move to next node (game)
        gameNode = gameNode->NextSibling();
    }

    // sort games
    std::sort(gl.games.begin(), gl.games.end(), sortGameByName);
    // sort lists
    std::sort(gl.systems.begin(), gl.systems.end(), sortByName);
    std::sort(gl.editors.begin(), gl.editors.end(), sortByName);
    std::sort(gl.developers.begin(), gl.developers.end(), sortByName);
    std::sort(gl.players.begin(), gl.players.end(), sortByName);
    std::sort(gl.ratings.begin(), gl.ratings.end(), sortByName);
    std::sort(gl.topstaffs.begin(), gl.topstaffs.end(), sortByName);
    std::sort(gl.rotations.begin(), gl.rotations.end(), sortByName);
    std::sort(gl.resolutions.begin(), gl.resolutions.end(), sortByName);
    std::sort(gl.dates.begin(), gl.dates.end(), sortByName);
    std::sort(gl.genres.begin(), gl.genres.end(), sortByName);
    // default lists values
    gl.systems.insert(gl.systems.begin(), "All");
    gl.editors.insert(gl.editors.begin(), "All");
    gl.developers.insert(gl.developers.begin(), "All");
    gl.players.insert(gl.players.begin(), "All");
    gl.ratings.insert(gl.ratings.begin(), "All");
    gl.topstaffs.insert(gl.topstaffs.begin(), "All");
    gl.rotations.insert(gl.rotations.begin(), "All");
    gl.resolutions.insert(gl.resolutions.begin(), "All");
    gl.dates.insert(gl.dates.begin(), "All");
    gl.genres.insert(gl.genres.begin(), "All");

    return gl;
}

std::vector<Game>
Api::gameListFilter(const std::vector<Game> &games, bool available, bool clones,
                    const std::string &system, const std::string &editor, const std::string &developer,
                    const std::string &player, const std::string &rating, const std::string &topstaff,
                    const std::string &rotation, const std::string &resolution, const std::string &date,
                    const std::string &genre) {

    std::vector<Game> newGameList;

    std::copy_if(games.begin(), games.end(), std::back_inserter(newGameList),
                 [available, clones, system, editor, developer, player, rating,
                         topstaff, rotation, resolution, date, genre](const Game &game) {
                     return (available || game.available)
                            && (clones || game.cloneof == "0")
                            && (system == "All" || game.system.text == system)
                            && (editor == "All" || game.editor.text == editor)
                            && (developer == "All" || game.developer.text == developer)
                            && (player == "All" || game.players == player)
                            && (rating == "All" || game.rating >= rating)
                            && (topstaff == "All" || game.topstaff == topstaff)
                            && (rotation == "All" || game.rotation == rotation)
                            && (resolution == "All" || game.resolution == resolution)
                            && (date == "All" || game.getDate(Game::Country::WOR).text >= date)
                            && (genre == "All" || game.getGenre(Game::Language::EN).text == genre);
                 });

    return newGameList;
}

Game Api::parseGame(XMLNode *gameNode, const std::string &romName) {

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
    if (game.path.empty()) {
        game.path = romName;
    }
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
    if (element) {
        XMLNode *node = element->FirstChildElement("date");
        while (node) {
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

bool Api::GameList::save(const std::string &dstPath) {

    XMLDocument doc;

    XMLDeclaration *dec = doc.NewDeclaration();
    doc.InsertFirstChild(dec);

    XMLNode *pRoot = doc.NewElement("Data");
    doc.InsertEndChild(pRoot);

    XMLElement *pGames = doc.NewElement("jeux");
    pRoot->InsertEndChild(pGames);

    for (const auto &game : games) {
        // screenscraper / emulationstation
        XMLElement *gameElement = doc.NewElement("jeu");
        gameElement->SetAttribute("id", game.id.c_str());
        gameElement->SetAttribute("romid", game.romid.c_str());
        gameElement->SetAttribute("notgame", game.notgame.c_str());
        gameElement->SetAttribute("source", game.source.c_str());
        // emulationstation
        XMLElement *elem = doc.NewElement("path");
        elem->SetText(game.path.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *names = doc.NewElement("noms");
        for (const auto &name : game.names) {
            XMLElement *n = doc.NewElement("nom");
            n->SetAttribute("region", name.country.c_str());
            n->SetText(name.text.c_str());
            names->InsertEndChild(n);
        }
        gameElement->InsertEndChild(names);
        // emulationstation
        elem = doc.NewElement("name");
        if (!game.names.empty()) {
            elem->SetText(game.getName(Game::Country::WOR).text.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *countries = doc.NewElement("regions");
        for (const auto &country : game.countries) {
            XMLElement *n = doc.NewElement("region");
            n->SetText(country.c_str());
            countries->InsertEndChild(n);
        }
        gameElement->InsertEndChild(countries);
        // screenscraper
        elem = doc.NewElement("cloneof");
        if (!game.cloneof.empty()) {
            elem->SetText(game.cloneof.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("systeme");
        if (!game.system.id.empty()) {
            elem->SetAttribute("id", game.system.id.c_str());
            elem->SetText(game.system.text.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *synopses = doc.NewElement("synopsis");
        for (const auto &synopsis : game.synopses) {
            XMLElement *n = doc.NewElement("synopsis");
            n->SetAttribute("langue", synopsis.language.c_str());
            n->SetText(synopsis.text.c_str());
            synopses->InsertEndChild(n);
        }
        gameElement->InsertEndChild(synopses);
        // emulationstation
        elem = doc.NewElement("desc");
        if (!game.synopses.empty()) {
            elem->SetText(game.synopses.at(0).text.c_str());
            elem->SetText(game.getSynopsis(Game::Language::EN).text.c_str());
        }
        gameElement->InsertEndChild(elem);
        // image
        elem = doc.NewElement("image");
        Game::Media image = game.getMedia(Game::Media::Type::SS, Game::Country::SS);
        if (!image.url.empty()) {
            elem->SetText(("media/images/"
                           + game.path.substr(0, game.path.find_last_of('.') + 1) + image.format).c_str());
        }
        gameElement->InsertEndChild(elem);
        // thumbnail
        elem = doc.NewElement("thumbnail");
        Game::Media thumbnail = game.getMedia(Game::Media::Type::Box3D, Game::Country::SS);
        if (!thumbnail.url.empty()) {
            elem->SetText(("media/box3d/"
                           + game.path.substr(0, game.path.find_last_of('.') + 1) + thumbnail.format).c_str());
        }
        gameElement->InsertEndChild(elem);
        // video
        elem = doc.NewElement("video");
        Game::Media video = game.getMedia(Game::Media::Type::Video, Game::Country::ALL);
        if (!video.url.empty()) {
            elem->SetText(("media/videos/"
                           + game.path.substr(0, game.path.find_last_of('.') + 1) + video.format).c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *_dates = doc.NewElement("dates");
        for (const auto &date : game.dates) {
            XMLElement *n = doc.NewElement("date");
            n->SetAttribute("region", date.country.c_str());
            n->SetText(date.text.c_str());
            _dates->InsertEndChild(n);
        }
        gameElement->InsertEndChild(_dates);
        // emulationstation
        elem = doc.NewElement("releasedate");
        if (!game.dates.empty()) {
            elem->SetText(game.dates.at(0).text.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("developpeur");
        elem->SetAttribute("id", game.developer.id.c_str());
        elem->SetText(game.developer.text.c_str());
        gameElement->InsertEndChild(elem);
        // emulationstation
        elem = doc.NewElement("developer");
        elem->SetText(game.developer.text.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("editeur");
        elem->SetAttribute("id", game.editor.id.c_str());
        elem->SetText(game.editor.text.c_str());
        gameElement->InsertEndChild(elem);
        // emulationstation
        elem = doc.NewElement("publisher");
        elem->SetText(game.editor.text.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *_genres = doc.NewElement("genres");
        for (const auto &genre : game.genres) {
            XMLElement *n = doc.NewElement("genre");
            n->SetAttribute("id", genre.id.c_str());
            n->SetAttribute("principale", genre.main.c_str());
            n->SetAttribute("parentid", genre.parentid.c_str());
            n->SetAttribute("langue", genre.language.c_str());
            n->SetText(genre.text.c_str());
            _genres->InsertEndChild(n);
        }
        gameElement->InsertEndChild(_genres);
        // emulationstation
        elem = doc.NewElement("genre");
        if (!game.genres.empty()) {
            elem->SetText(game.genres.at(0).main.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("joueurs");
        elem->SetText(game.players.c_str());
        gameElement->InsertEndChild(elem);
        // emulationstation
        elem = doc.NewElement("players");
        elem->SetText(game.players.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("topstaff");
        elem->SetText(game.topstaff.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("rotation");
        elem->SetText(game.rotation.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("resolution");
        elem->SetText(game.resolution.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("inputs");
        elem->SetText(game.inputs.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("colors");
        elem->SetText(game.colors.c_str());
        gameElement->InsertEndChild(elem);
        // add game element
        pGames->InsertEndChild(gameElement);
    }

    XMLError e = doc.SaveFile(dstPath.c_str());
    if (e != XML_SUCCESS) {
        SS_PRINT("GameList::save: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return false;
    }

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

bool Api::sortByName(const std::string &g1, const std::string &g2) {
    return strcasecmp(g1.c_str(), g2.c_str()) <= 0;
}

bool Api::sortGameByName(const Game &g1, const Game &g2) {
    return strcasecmp(g1.getName().text.c_str(), g2.getName().text.c_str()) <= 0;
}
