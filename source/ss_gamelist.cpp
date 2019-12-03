//
// Created by cpasjuste on 03/12/2019.
//

#include <algorithm>
#include "ss_api.h"
#include "ss_io.h"
#include "ss_gamelist.h"

using namespace ss_api;
using namespace tinyxml2;

GameList::GameList(const std::string &xmlPath, const std::string &rPath) {

    XMLDocument doc;
    std::vector<std::string> files;

    xml = xmlPath;
    XMLError e = doc.LoadFile(xmlPath.c_str());
    if (e != XML_SUCCESS) {
        SS_PRINT("Api::gameList: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return;
    }

    XMLNode *pRoot = doc.FirstChildElement("Data");
    if (pRoot == nullptr) {
        // emulationstation format
        pRoot = doc.FirstChildElement("gameList");
        if (pRoot == nullptr) {
            SS_PRINT("Api::parseGameSearch: wrong xml format: \'Data\' or \'gameList\' tag not found\n");
            return;
        }
    }

    XMLNode *gamesNode = pRoot->FirstChildElement("jeux");
    if (gamesNode == nullptr) {
        // emulationstation format
        gamesNode = pRoot;
    }

    XMLNode *gameNode = gamesNode->FirstChildElement("jeu");
    if (gameNode == nullptr) {
        gameNode = gamesNode->FirstChildElement("game");
    }

    romPath = rPath;
    if (!romPath.empty()) {
        files = Io::getDirList(romPath);
    }

    while (gameNode != nullptr) {
        Game game = Api::parseGame(gameNode);
        // is rom available?
        auto p = std::find(files.begin(), files.end(), game.path);
        if (p != files.end()) {
            game.available = true;
        }
        // add stuff for later filtering
        p = std::find(systems.begin(), systems.end(), game.system.text);
        if (p == systems.end()) {
            systems.emplace_back(game.system.text);
        }
        p = std::find(editors.begin(), editors.end(), game.editor.text);
        if (p == editors.end()) {
            editors.emplace_back(game.editor.text);
        }
        p = std::find(developers.begin(), developers.end(), game.developer.text);
        if (p == developers.end()) {
            developers.emplace_back(game.developer.text);
        }
        p = std::find(players.begin(), players.end(), game.players);
        if (p == players.end()) {
            players.emplace_back(game.players);
        }
        p = std::find(ratings.begin(), ratings.end(), game.rating);
        if (p == ratings.end()) {
            ratings.emplace_back(game.rating);
        }
        p = std::find(topstaffs.begin(), topstaffs.end(), game.topstaff);
        if (p == topstaffs.end()) {
            topstaffs.emplace_back(game.topstaff);
        }
        p = std::find(rotations.begin(), rotations.end(), game.rotation);
        if (p == rotations.end()) {
            rotations.emplace_back(game.rotation);
        }
        p = std::find(resolutions.begin(), resolutions.end(), game.resolution);
        if (p == resolutions.end()) {
            resolutions.emplace_back(game.resolution);
        }
        if (!game.dates.empty()) {
            std::string date = game.getDate(Game::Country::WOR).text;
            p = std::find(dates.begin(), dates.end(), date);
            if (p == dates.end()) {
                dates.emplace_back(date);
            }
        }
        if (!game.genres.empty()) {
            std::string genre = game.getGenre(Game::Language::EN).text;
            p = std::find(genres.begin(), genres.end(), genre);
            if (p == genres.end()) {
                genres.emplace_back(genre);
            }
        }
        // add game to game list
        games.emplace_back(game);
        // move to next node (game)
        gameNode = gameNode->NextSibling();
    }

    // sort games
    std::sort(games.begin(), games.end(), Api::sortGameByName);
    // sort lists
    std::sort(systems.begin(), systems.end(), Api::sortByName);
    std::sort(editors.begin(), editors.end(), Api::sortByName);
    std::sort(developers.begin(), developers.end(), Api::sortByName);
    std::sort(players.begin(), players.end(), Api::sortByName);
    std::sort(ratings.begin(), ratings.end(), Api::sortByName);
    std::sort(topstaffs.begin(), topstaffs.end(), Api::sortByName);
    std::sort(rotations.begin(), rotations.end(), Api::sortByName);
    std::sort(resolutions.begin(), resolutions.end(), Api::sortByName);
    std::sort(dates.begin(), dates.end(), Api::sortByName);
    std::sort(genres.begin(), genres.end(), Api::sortByName);
    // default lists values
    systems.insert(systems.begin(), "All");
    editors.insert(editors.begin(), "All");
    developers.insert(developers.begin(), "All");
    players.insert(players.begin(), "All");
    ratings.insert(ratings.begin(), "All");
    topstaffs.insert(topstaffs.begin(), "All");
    rotations.insert(rotations.begin(), "All");
    resolutions.insert(resolutions.begin(), "All");
    dates.insert(dates.begin(), "All");
    genres.insert(genres.begin(), "All");
}

GameList GameList::filter(bool available, bool clones, const std::string &system, const std::string &editor,
                          const std::string &developer, const std::string &player, const std::string &rating,
                          const std::string &topstaff, const std::string &rotation,
                          const std::string &resolution, const std::string &date, const std::string &genre) {

    GameList gameList;
    gameList.xml = xml;
    gameList.romPath = romPath;
    gameList.systems = systems;
    gameList.editors = editors;
    gameList.developers = developers;
    gameList.players = players;
    gameList.ratings = ratings;
    gameList.topstaffs = topstaffs;
    gameList.rotations = rotations;
    gameList.resolutions = resolutions;
    gameList.dates = dates;
    gameList.genres = genres;

    std::copy_if(games.begin(), games.end(), std::back_inserter(gameList.games),
                 [available, clones, system, editor, developer, player, rating,
                         topstaff, rotation, resolution, date, genre](const Game &game) {
                     // TODO: use integer for rating, resolution and date
                     return (!available || (available && game.available))
                            && (clones || game.cloneof == "0")
                            && (system == "All" || game.system.text == system)
                            && (editor == "All" || game.editor.text == editor)
                            && (developer == "All" || game.developer.text == developer)
                            && (player == "All" || game.players == player)
                            && (rating == "All" || game.rating == rating)
                            && (topstaff == "All" || game.topstaff == topstaff)
                            && (rotation == "All" || game.rotation == rotation)
                            && (resolution == "All" || game.resolution == resolution)
                            && (date == "All" || game.getDate(Game::Country::WOR).text == date)
                            && (genre == "All" || game.getGenre(Game::Language::EN).text == genre);
                 });

    return gameList;
}

int GameList::getAvailableCount() {

    return std::count_if(games.begin(), games.end(), [](const Game &game) {
        return game.available;
    });
}

bool GameList::save(const std::string &dstPath) {

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

        // screenscraper medias
        XMLElement *medias = doc.NewElement("medias");
        for (const auto &media : game.medias) {
            XMLElement *n = doc.NewElement("media");
            n->SetAttribute("parent", media.parent.c_str());
            n->SetAttribute("type", media.type.c_str());
            n->SetAttribute("region", media.country.c_str());
            n->SetAttribute("crc", media.crc.c_str());
            n->SetAttribute("md5", media.md5.c_str());
            n->SetAttribute("sha1", media.sha1.c_str());
            n->SetAttribute("format", media.format.c_str());
            n->SetAttribute("support", media.support.c_str());
            if (!media.url.empty()) {
                if (media.url.rfind("http", 0) == 0) {
                    n->SetText(("media/" + media.type + "/"
                                + game.path.substr(0, game.path.find_last_of('.') + 1) + media.format).c_str());
                } else {
                    n->SetText(media.url.c_str());
                }
            }
            medias->InsertEndChild(n);
        }
        gameElement->InsertEndChild(medias);

        // emulationstation medias
        // image
        elem = doc.NewElement("image");
        Game::Media image = game.getMedia("sstitle", Game::Country::SS);
        if (!image.url.empty()) {
            if (image.url.rfind("http", 0) == 0) {
                elem->SetText(("media/ss/"
                               + game.path.substr(0, game.path.find_last_of('.') + 1) + image.format).c_str());
            } else {
                elem->SetText(image.url.c_str());
            }
        }
        gameElement->InsertEndChild(elem);
        // thumbnail
        elem = doc.NewElement("thumbnail");
        Game::Media thumbnail = game.getMedia("ss", Game::Country::SS);
        if (!thumbnail.url.empty()) {
            if (thumbnail.url.rfind("http", 0) == 0) {
                elem->SetText(("media/sstitle/"
                               + game.path.substr(0, game.path.find_last_of('.') + 1) + thumbnail.format).c_str());
            } else {
                elem->SetText(thumbnail.url.c_str());
            }
        }
        gameElement->InsertEndChild(elem);
        // video
        elem = doc.NewElement("video");
        Game::Media video = game.getMedia("video", Game::Country::ALL);
        if (!video.url.empty()) {
            if (video.url.rfind("http", 0) == 0) {
                elem->SetText(("media/video/"
                               + game.path.substr(0, game.path.find_last_of('.') + 1) + video.format).c_str());
            } else {
                elem->SetText(video.url.c_str());
            }
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
            elem->SetText(game.getGenre(Game::Language::EN).text.c_str());
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
        elem = doc.NewElement("note");
        elem->SetText(game.rating.c_str());
        gameElement->InsertEndChild(elem);
        // emulationstation
        elem = doc.NewElement("rating");
        elem->SetText(game.rating.c_str());
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
        elem = doc.NewElement("controles");
        elem->SetText(game.inputs.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("couleurs");
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

// pfba: fix screenscraper missing clonesof
bool GameList::fixClones(const std::string &fbaGamelist) {

    ///
    /// build fbneo game list START
    ///
    std::vector<Game> fbaList;
    XMLDocument doc;
    XMLError e = doc.LoadFile(fbaGamelist.c_str());
    if (e != XML_SUCCESS) {
        SS_PRINT_RED("Api::gameListFixClones: could not load dat: %s (%s)\n",
                     fbaGamelist.c_str(), tinyxml2::XMLDocument::ErrorIDToName(e));
        doc.Clear();
        return false;
    }

    XMLNode *pRoot = doc.FirstChildElement("datafile");
    if (pRoot == nullptr) {
        SS_PRINT_RED("Api::gameListFixClones: wrong xml format: \'datafile\' tag not found\n");
        doc.Clear();
        return false;
    }

    XMLNode *gameNode = pRoot->FirstChildElement("game");
    if (gameNode == nullptr) {
        SS_PRINT_RED("Api::gameListFixClones: no \'game\' node found\n");
        doc.Clear();
        return false;
    }

    while (gameNode != nullptr) {
        Game game;
        game.names.emplace_back("wor", Api::getXmlText(gameNode->FirstChildElement("description")));
        game.path = Api::getXmlAttribute(gameNode->ToElement(), "name");
        if (!game.path.empty()) {
            game.path += ".zip";
        }
        game.cloneof = Api::getXmlAttribute(gameNode->ToElement(), "cloneof");
        fbaList.emplace_back(game);
        gameNode = gameNode->NextSibling();
    }

    doc.Clear();

    ///
    /// build fbneo game list END
    ///

    for (size_t i = 0; i < games.size(); i++) {

        // screenscraper game "cloneof" is set, continue
        if (games.at(i).cloneof != "0") {
            continue;
        }

        // search fba list for zip name (path), as game name may differ
        std::string zipName = games.at(i).path;
        auto fbaGame = std::find_if(fbaList.begin(), fbaList.end(), [zipName](const Game &g) {
            return zipName == g.path;
        });
        // screenscraper game not found in fbneo dat, continue...
        if (fbaGame == fbaList.end()) {
            continue;
        }

        // game is not a clone, even in fba dat, continue...
        if ((*fbaGame).cloneof.empty()) {
            continue;
        }

        // we found a bad/wrong screenscaper "cloneof", find screenscraper parent id
        std::string parentZipName = (*fbaGame).cloneof + ".zip";
        auto sscrapParent = std::find_if(games.begin(), games.end(),
                                         [parentZipName](const Game &g) {
                                             return parentZipName == g.path;
                                         });
        // screenscraper parent not found (?!), continue...
        if (sscrapParent == games.end()) {
            continue;
        }

        // fix screenscraper cloneof !
        games.at(i).cloneof = (*sscrapParent).romid;

        // debug
        if (games.at(i).cloneof == games.at(i).romid) {
            SS_PRINT_RED("clone: %s (%s, id: %s) => parent: %s (%s, id: %s): cloneof can't equal romid\n",
                         games.at(i).getName().text.c_str(),
                         games.at(i).path.c_str(),
                         games.at(i).romid.c_str(),
                         (*sscrapParent).getName().text.c_str(),
                         (*sscrapParent).path.c_str(),
                         (*sscrapParent).romid.c_str());
        } else {
            SS_PRINT("fix: clone: %s (%s, id: %s) => parent: %s (%s, id: %s)\n",
                     games.at(i).getName().text.c_str(),
                     games.at(i).path.c_str(),
                     games.at(i).romid.c_str(),
                     (*sscrapParent).getName().text.c_str(),
                     (*sscrapParent).path.c_str(),
                     (*sscrapParent).romid.c_str());
            // screenscrap db
            //SS_PRINT("UPDATE Roms SET cloneof=%s WHERE id=%s;\n",
            //         (*sscrapParent).romid.c_str(), games.at(i).romid.c_str());
        }
    }

    return true;
}
