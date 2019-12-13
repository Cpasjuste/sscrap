//
// Created by cpasjuste on 03/12/2019.
//

#include <algorithm>
#include "ss_api.h"

using namespace ss_api;

GameList::GameList(const std::string &xmlPath, const std::string &rPath) {
    append(xmlPath, rPath);
}

bool GameList::append(const std::string &xmlPath, const std::string &rPath) {

    tinyxml2::XMLDocument doc;
    std::vector<std::string> files;

    xml = xmlPath;
    tinyxml2::XMLError e = doc.LoadFile(xmlPath.c_str());
    if (e != tinyxml2::XML_SUCCESS) {
        SS_PRINT("GameList: %s\n", doc.ErrorName());
        doc.Clear();
        return false;
    }

    tinyxml2::XMLNode *pRoot = doc.FirstChildElement("Data");
    if (pRoot == nullptr) {
        pRoot = doc.FirstChildElement("gameList");
        if (pRoot == nullptr) {
            pRoot = doc.FirstChildElement("datafile");
            if (pRoot == nullptr) {
                SS_PRINT("GameList: wrong xml format: \'Data\', \'gameList\' or \'datafile\' tag not found\n");
                format = Format::Unknown;
                doc.Clear();
                return false;
            }
            format = Format::FbNeo;
        } else {
            format = Format::EmulationStation;
        }
    } else {
        format = Format::ScreenScrapper;
    }

    tinyxml2::XMLNode *gamesNode = pRoot->FirstChildElement("jeux");
    if (gamesNode == nullptr) {
        // es/fbneo format
        gamesNode = pRoot;
    }

    tinyxml2::XMLNode *gameNode = gamesNode->FirstChildElement("jeu");
    if (gameNode == nullptr) {
        // es/fbneo format
        gameNode = gamesNode->FirstChildElement("game");
    }

    if (!rPath.empty()) {
        romPaths.emplace_back(rPath);
        files = Io::getDirList(rPath);
    }

    while (gameNode != nullptr) {
        Game game;
        Game::parseGame(&game, gameNode, "", format);
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
    if (systems.empty() || systems.at(0) != "All") {
        systems.insert(systems.begin(), "All");
    }
    if (editors.empty() || editors.at(0) != "All") {
        editors.insert(editors.begin(), "All");
    }
    if (developers.empty() || developers.at(0) != "All") {
        developers.insert(developers.begin(), "All");
    }
    if (players.empty() || players.at(0) != "All") {
        players.insert(players.begin(), "All");
    }
    if (ratings.empty() || ratings.at(0) != "All") {
        ratings.insert(ratings.begin(), "All");
    }
    if (topstaffs.empty() || topstaffs.at(0) != "All") {
        topstaffs.insert(topstaffs.begin(), "All");
    }
    if (rotations.empty() || rotations.at(0) != "All") {
        rotations.insert(rotations.begin(), "All");
    }
    if (resolutions.empty() || resolutions.at(0) != "All") {
        resolutions.insert(resolutions.begin(), "All");
    }
    if (dates.empty() || dates.at(0) != "All") {
        dates.insert(dates.begin(), "All");
    }
    if (genres.empty() || genres.at(0) != "All") {
        genres.insert(genres.begin(), "All");
    }

    return true;
}

bool GameList::save(const std::string &dstPath, const Game::Language &language,
                    const Format &fmt, const std::vector<std::string> &mediaList) {

    tinyxml2::XMLDocument doc;

    tinyxml2::XMLDeclaration *dec = doc.NewDeclaration();
    doc.InsertFirstChild(dec);

    tinyxml2::XMLNode *pRoot = fmt == Format::EmulationStation ?
                               doc.NewElement("gameList") : doc.NewElement("Data");
    doc.InsertEndChild(pRoot);

    tinyxml2::XMLElement *pGames = fmt == Format::EmulationStation ?
                                   pRoot->ToElement() : doc.NewElement("jeux");
    if (fmt == Format::ScreenScrapper) {
        pRoot->InsertEndChild(pGames);
    }

    // sort games
    std::sort(games.begin(), games.end(), Api::sortGameByName);

    for (const auto &game : games) {
        if (fmt == Format::EmulationStation) {
            tinyxml2::XMLElement *gameElement = doc.NewElement("game");
            if (!game.id.empty()) {
                gameElement->SetAttribute("id", game.id.c_str());
            }
            if (!game.source.empty()) {
                gameElement->SetAttribute("source", game.source.c_str());
            }
            Api::addXmlElement(&doc, gameElement, "path", game.path);
            Api::addXmlElement(&doc, gameElement, "name", game.getName().text);
            Api::addXmlElement(&doc, gameElement, "desc", game.getSynopsis(language).text);
            Api::addXmlElement(&doc, gameElement, "rating", game.rating);
            Api::addXmlElement(&doc, gameElement, "releasedate", game.getDate().text);
            Api::addXmlElement(&doc, gameElement, "developer", game.developer.text);
            Api::addXmlElement(&doc, gameElement, "publisher", game.editor.text);
            Api::addXmlElement(&doc, gameElement, "genre", game.getGenre(language).text);
            Api::addXmlElement(&doc, gameElement, "players", game.players);

            const std::vector<std::string> names = {"image", "thumbnail", "video"};
            for (size_t i = 0; i < names.size(); i++) {
                if (mediaList.size() > names.size()) {
                    break;
                }
                Game::Media media = game.getMedia(mediaList.at(i), Game::Country::SS);
                if (!media.url.empty()) {
                    std::string mediaPath = media.url;
                    if (mediaPath.rfind("http", 0) == 0) {
                        mediaPath = "media/" + mediaList.at(i) + "/"
                                    + game.path.substr(0, game.path.find_last_of('.') + 1)
                                    + media.format;
                    }
                    Api::addXmlElement(&doc, gameElement, names.at(i), mediaPath);
                }
            }

            pGames->InsertEndChild(gameElement);
        } else {
            tinyxml2::XMLElement *elem;
            tinyxml2::XMLElement *gameElement = doc.NewElement("jeu");
            if (!game.id.empty()) {
                gameElement->SetAttribute("id", game.id.c_str());
            }
            if (!game.romid.empty()) {
                gameElement->SetAttribute("romid", game.romid.c_str());
            }
            if (!game.notgame.empty()) {
                gameElement->SetAttribute("notgame", game.notgame.c_str());
            }
            Api::addXmlElement(&doc, gameElement, "path", game.path);

            tinyxml2::XMLElement *names = doc.NewElement("noms");
            for (const auto &name : game.names) {
                tinyxml2::XMLElement *n = doc.NewElement("nom");
                n->SetAttribute("region", name.country.c_str());
                n->SetText(name.text.c_str());
                names->InsertEndChild(n);
            }
            gameElement->InsertEndChild(names);

            if (!game.countries.empty()) {
                tinyxml2::XMLElement *countries = doc.NewElement("regions");
                for (const auto &country : game.countries) {
                    tinyxml2::XMLElement *n = doc.NewElement("region");
                    n->SetText(country.c_str());
                    countries->InsertEndChild(n);
                }
                gameElement->InsertEndChild(countries);
            }

            Api::addXmlElement(&doc, gameElement, "cloneof", game.cloneof);

            elem = doc.NewElement("systeme");
            if (!game.system.id.empty()) {
                elem->SetAttribute("id", game.system.id.c_str());
                elem->SetAttribute("parentid", game.system.parentId.c_str());
                elem->SetText(game.system.text.c_str());
            }
            gameElement->InsertEndChild(elem);

            if (!game.synopses.empty()) {
                tinyxml2::XMLElement *synopses = doc.NewElement("synopsis");
                for (const auto &synopsis : game.synopses) {
                    if (synopsis.language != Api::toString(language)) {
                        continue;
                    }
                    tinyxml2::XMLElement *n = doc.NewElement("synopsis");
                    n->SetAttribute("langue", synopsis.language.c_str());
                    n->SetText(synopsis.text.c_str());
                    synopses->InsertEndChild(n);
                }
                gameElement->InsertEndChild(synopses);
            }

            if (!game.medias.empty()) {
                tinyxml2::XMLElement *mediasElement = doc.NewElement("medias");
                for (const auto &mediaType : mediaList) {
                    Game::Media media = game.getMedia(mediaType, Game::Country::SS);
                    if (!media.url.empty()) {
                        tinyxml2::XMLElement *n = doc.NewElement("media");
                        n->SetAttribute("parent", media.parent.c_str());
                        n->SetAttribute("type", media.type.c_str());
                        n->SetAttribute("region", media.country.c_str());
                        n->SetAttribute("crc", media.crc.c_str());
                        n->SetAttribute("md5", media.md5.c_str());
                        n->SetAttribute("sha1", media.sha1.c_str());
                        n->SetAttribute("format", media.format.c_str());
                        n->SetAttribute("support", media.support.c_str());
                        if (media.url.rfind("http", 0) == 0) {
                            n->SetText(("media/" + media.type + "/"
                                        + game.path.substr(0, game.path.find_last_of('.') + 1) +
                                        media.format).c_str());
                        } else {
                            n->SetText(media.url.c_str());
                        }
                        mediasElement->InsertEndChild(n);
                    }
                }
                gameElement->InsertEndChild(mediasElement);
            }

            if (!game.dates.empty()) {
                tinyxml2::XMLElement *_dates = doc.NewElement("dates");
                for (const auto &date : game.dates) {
                    tinyxml2::XMLElement *n = doc.NewElement("date");
                    n->SetAttribute("region", date.country.c_str());
                    n->SetText(date.text.c_str());
                    _dates->InsertEndChild(n);
                }
                gameElement->InsertEndChild(_dates);
            }

            if (!game.developer.text.empty()) {
                elem = doc.NewElement("developpeur");
                elem->SetAttribute("id", game.developer.id.c_str());
                elem->SetText(game.developer.text.c_str());
                gameElement->InsertEndChild(elem);
            }

            if (!game.editor.text.empty()) {
                elem = doc.NewElement("editeur");
                elem->SetAttribute("id", game.editor.id.c_str());
                elem->SetText(game.editor.text.c_str());
                gameElement->InsertEndChild(elem);
            }

            if (!game.genres.empty()) {
                tinyxml2::XMLElement *_genres = doc.NewElement("genres");
                for (const auto &genre : game.genres) {
                    if (genre.language != Api::toString(language)) {
                        continue;
                    }
                    tinyxml2::XMLElement *n = doc.NewElement("genre");
                    n->SetAttribute("id", genre.id.c_str());
                    n->SetAttribute("principale", genre.main.c_str());
                    n->SetAttribute("parentid", genre.parentid.c_str());
                    n->SetAttribute("langue", genre.language.c_str());
                    n->SetText(genre.text.c_str());
                    _genres->InsertEndChild(n);
                }
                gameElement->InsertEndChild(_genres);
            }

            Api::addXmlElement(&doc, gameElement, "joueurs", game.players);
            Api::addXmlElement(&doc, gameElement, "topstaff", game.topstaff);
            Api::addXmlElement(&doc, gameElement, "note", game.rating);
            Api::addXmlElement(&doc, gameElement, "rotation", game.rotation);
            Api::addXmlElement(&doc, gameElement, "resolution", game.resolution);
            Api::addXmlElement(&doc, gameElement, "controles", game.inputs);
            Api::addXmlElement(&doc, gameElement, "couleurs", game.colors);

            // add game element
            pGames->InsertEndChild(gameElement);
        }
    }

    tinyxml2::XMLError e = doc.SaveFile(dstPath.c_str());
    if (e != tinyxml2::XML_SUCCESS) {
        SS_PRINT("GameList::save: %s\n", doc.ErrorName());
        doc.Clear();
        return false;
    }

    doc.Clear();

    return true;
}

GameList GameList::filter(bool available, bool clones, const std::string &system, const std::string &editor,
                          const std::string &developer, const std::string &player, const std::string &rating,
                          const std::string &topstaff, const std::string &rotation,
                          const std::string &resolution, const std::string &date, const std::string &genre) {

    GameList gameList;
    gameList.xml = xml;
    gameList.romPaths = romPaths;
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
                            && (clones || !game.isClone())
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

Game GameList::findByRomId(const std::string &romId) {

    auto it = std::find_if(games.begin(), games.end(), [romId](const Game &game) {
        return game.romid == romId;
    });

    if (it != games.end()) {
        return *it;
    }

    return Game();
}

Game GameList::findByPath(const std::string &path) {

    auto it = std::find_if(games.begin(), games.end(), [path](const Game &game) {
        return game.path == path;
    });

    if (it != games.end()) {
        return *it;
    }

    return Game();
}

bool GameList::exist(const std::string &romId) {

    auto it = std::find_if(games.begin(), games.end(), [romId](const Game &game) {
        return game.romid == romId;
    });

    return it != games.end();
}

bool GameList::remove(const std::string &romId) {

    auto it = std::find_if(games.begin(), games.end(), [romId](const Game &game) {
        return game.romid == romId;
    });

    if (it != games.end()) {
        games.erase(it);
        return true;
    }

    return false;
}

int GameList::getAvailableCount() {

    return std::count_if(games.begin(), games.end(), [](const Game &game) {
        return game.available;
    });
}
