//
// Created by cpasjuste on 03/12/2019.
//

#include <algorithm>
#include <cmath>

#include "ss_api.h"

using namespace ss_api;

GameList::GameList(const std::string &xmlPath, const std::string &rPath, bool sort, bool addUnknownFiles) {
    append(xmlPath, rPath, sort, addUnknownFiles);
}

bool GameList::append(const std::string &xmlPath, const std::string &rPath, bool sort, bool addUnknownFiles) {

    tinyxml2::XMLDocument doc;
    std::vector<Io::File> files;

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
        if (addUnknownFiles) {
            files = Io::getDirList(rPath, false, {});
        } else {
            files = Io::getDirList(rPath, false);
        }
    }

    while (gameNode != nullptr) {
        Game game;
        Game::parseGame(&game, gameNode, "", format);
        // set game "real path", minus filename (for pFBN)
        game.romsPath = rPath;
        // is rom available?
        //auto p = std::find(files.begin(), files.end(), game.path);
        const std::string n = game.path;
        auto it = std::find_if(files.begin(), files.end(), [n](const Io::File &f) {
            return f.name == n;
        });
        if (it != files.end()) {
            game.available = true;
            files.erase(it);
        }
        // add stuff for later filtering
        std::string system = game.system.text.empty() ? "Unknown" : game.system.text;
        auto it2 = std::find(systems.begin(), systems.end(), system);
        if (it2 == systems.end()) {
            systems.emplace_back(system);
        }
        std::string editor = game.editor.text.empty() ? "Unknown" : game.editor.text;
        it2 = std::find(editors.begin(), editors.end(), editor);
        if (it2 == editors.end()) {
            editors.emplace_back(editor);
        }
        std::string developer = game.developer.text.empty() ? "Unknown" : game.developer.text;
        it2 = std::find(developers.begin(), developers.end(), developer);
        if (it2 == developers.end()) {
            developers.emplace_back(developer);
        }
        std::string _players = game.players.empty() ? "Unknown" : game.players;
        it2 = std::find(players.begin(), players.end(), _players);
        if (it2 == players.end()) {
            players.emplace_back(_players);
        }
        std::string rating = std::to_string(game.rating);
        it2 = std::find(ratings.begin(), ratings.end(), rating);
        if (it2 == ratings.end()) {
            ratings.emplace_back(rating);
        }
        std::string topStaff = std::to_string((int) game.topStaff);
        it2 = std::find(topStaffs.begin(), topStaffs.end(), topStaff);
        if (it2 == topStaffs.end()) {
            topStaffs.emplace_back(topStaff);
        }
        std::string rotation = std::to_string(game.rotation);
        it2 = std::find(rotations.begin(), rotations.end(), rotation);
        if (it2 == rotations.end()) {
            rotations.emplace_back(rotation);
        }
        std::string resolution = game.resolution.empty() ? "Unknown" : game.resolution;
        it2 = std::find(resolutions.begin(), resolutions.end(), resolution);
        if (it2 == resolutions.end()) {
            resolutions.emplace_back(resolution);
        }
        if (!game.dates.empty()) {
            std::string date = game.getDate(Game::Country::WOR).text;
            if (date.empty()) {
                date = "Unknown";
            }
            it2 = std::find(dates.begin(), dates.end(), date);
            if (it2 == dates.end()) {
                dates.emplace_back(date);
            }
        }
        if (!game.genres.empty()) {
            std::string genre = game.getGenre(Game::Language::EN).text;
            if (genre.empty()) {
                genre = "Unknown";
            }
            it2 = std::find(genres.begin(), genres.end(), genre);
            if (it2 == genres.end()) {
                genres.emplace_back(genre);
            }
        }
        // add game to game list
        games.emplace_back(game);
        // move to next node (game)
        gameNode = gameNode->NextSibling();
    }

    if (addUnknownFiles) {
        for (const auto &file: files) {
            Game game;
            game.path = file.name;
            game.romsPath = rPath;
            game.names.emplace_back(Game::Country::UNK, file.name);
            game.available = true;
            games.emplace_back(game);
        }
    }

    if (sort) {
        sortAlpha();
    }

    return true;
}

void GameList::sortAlpha(bool byZipName, bool gamesOnly) {
    // sort games
    if (byZipName) {
        std::sort(games.begin(), games.end(), Api::sortGameByPath);
    } else {
        std::sort(games.begin(), games.end(), Api::sortGameByName);
    }

    // sort lists
    if (!gamesOnly) {
        std::sort(systems.begin(), systems.end(), Api::sortByName);
        std::sort(editors.begin(), editors.end(), Api::sortByName);
        std::sort(developers.begin(), developers.end(), Api::sortByName);
        std::sort(players.begin(), players.end(), Api::sortByName);
        std::sort(ratings.begin(), ratings.end(), Api::sortByName);
        std::sort(topStaffs.begin(), topStaffs.end(), Api::sortByName);
        std::sort(rotations.begin(), rotations.end(), Api::sortByName);
        std::sort(resolutions.begin(), resolutions.end(), Api::sortByName);
        std::sort(dates.begin(), dates.end(), Api::sortByName);
        std::sort(genres.begin(), genres.end(), Api::sortByName);
    }
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

    for (const auto &game: games) {
        if (fmt == Format::EmulationStation) {
            tinyxml2::XMLElement *gameElement = doc.NewElement("game");
            if (game.id > 0) {
                gameElement->SetAttribute("id", game.id);
            }
            if (!game.source.empty()) {
                gameElement->SetAttribute("source", game.source.c_str());
            }
            Api::addXmlElement(&doc, gameElement, "path", "./" + game.path);
            Api::addXmlElement(&doc, gameElement, "name", game.getName().text);
            Api::addXmlElement(&doc, gameElement, "desc", game.getSynopsis(language).text);
            // TODO: is this recalbox only?
            if (game.rating > 0) {
                std::string rating = std::to_string((float) game.rating / 20.0f);
                Api::addXmlElement(&doc, gameElement, "rating", rating.substr(0, rating.find('.') + 3));
            }
            Api::addXmlElement(&doc, gameElement, "releasedate", game.getDate().text + "0101T000000");
            // TODO: is this recalbox only?
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
                        mediaPath = "./media/" + mediaList.at(i) + "/"
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
            if (game.id > 0) {
                gameElement->SetAttribute("id", game.id);
            }
            if (game.romId > 0) {
                gameElement->SetAttribute("romid", game.romId);
            }
            if (game.notGame) {
                gameElement->SetAttribute("notgame", game.notGame);
            }
            Api::addXmlElement(&doc, gameElement, "path", game.path);

            tinyxml2::XMLElement *names = doc.NewElement("noms");
            for (const auto &name: game.names) {
                tinyxml2::XMLElement *n = doc.NewElement("nom");
                n->SetAttribute("region", Api::toString(name.country).c_str());
                n->SetText(name.text.c_str());
                names->InsertEndChild(n);
            }
            gameElement->InsertEndChild(names);

#if 0
            if (!game.countries.empty()) {
                tinyxml2::XMLElement *countries = doc.NewElement("regions");
                for (const auto &country: game.countries) {
                    tinyxml2::XMLElement *n = doc.NewElement("region");
                    n->SetText(Api::toString(country).c_str());
                    countries->InsertEndChild(n);
                }
                gameElement->InsertEndChild(countries);
            }
#endif
            Api::addXmlElement(&doc, gameElement, "cloneof", game.cloneOf);

            elem = doc.NewElement("systeme");
            if (game.system.id > 0) {
                elem->SetAttribute("id", game.system.id);
                elem->SetAttribute("parentid", game.system.parentId);
                elem->SetText(game.system.text.c_str());
            }
            gameElement->InsertEndChild(elem);

            if (!game.synopses.empty()) {
                tinyxml2::XMLElement *synopses = doc.NewElement("synopsis");
                for (const auto &synopsis: game.synopses) {
                    if (synopsis.language != language) {
                        continue;
                    }
                    tinyxml2::XMLElement *n = doc.NewElement("synopsis");
                    n->SetAttribute("langue", Api::toString(synopsis.language).c_str());
                    n->SetText(synopsis.text.c_str());
                    synopses->InsertEndChild(n);
                }
                gameElement->InsertEndChild(synopses);
            }

            if (!game.medias.empty()) {
                tinyxml2::XMLElement *mediasElement = doc.NewElement("medias");
                for (const auto &mediaType: mediaList) {
                    Game::Media media = game.getMedia(mediaType, Game::Country::SS);
                    if (!media.url.empty()) {
                        tinyxml2::XMLElement *n = doc.NewElement("media");
                        n->SetAttribute("parent", media.parent.c_str());
                        n->SetAttribute("type", media.type.c_str());
                        n->SetAttribute("region", Api::toString(media.country).c_str());
                        //n->SetAttribute("crc", media.crc.c_str());
                        //n->SetAttribute("md5", media.md5.c_str());
                        //n->SetAttribute("sha1", media.sha1.c_str());
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
                Game::Date d = game.getDate();
                if (!d.text.empty()) {
                    tinyxml2::XMLElement *n = doc.NewElement("date");
                    n->SetAttribute("region", Api::toString(d.country).c_str());
                    n->SetText(d.text.c_str());
                    _dates->InsertEndChild(n);
                }
                gameElement->InsertEndChild(_dates);
            }

            if (!game.developer.text.empty()) {
                elem = doc.NewElement("developpeur");
                elem->SetAttribute("id", game.developer.id);
                elem->SetText(game.developer.text.c_str());
                gameElement->InsertEndChild(elem);
            }

            if (!game.editor.text.empty()) {
                elem = doc.NewElement("editeur");
                elem->SetAttribute("id", game.editor.id);
                elem->SetText(game.editor.text.c_str());
                gameElement->InsertEndChild(elem);
            }

            if (!game.genres.empty()) {
                tinyxml2::XMLElement *genresElement = doc.NewElement("genres");
                Game::Genre g = game.getGenre(language);
                if (!g.text.empty()) {
                    tinyxml2::XMLElement *n = doc.NewElement("genre");
                    n->SetAttribute("id", g.id);
                    n->SetAttribute("principale", g.mainId);
                    n->SetAttribute("parentid", g.parentId);
                    n->SetAttribute("langue", Api::toString(g.language).c_str());
                    n->SetText(g.text.c_str());
                    genresElement->InsertEndChild(n);
                }
                gameElement->InsertEndChild(genresElement);
            }

            Api::addXmlElement(&doc, gameElement, "joueurs", game.players);
            Api::addXmlElement(&doc, gameElement, "topstaff", game.topStaff ? "1" : "0");
            Api::addXmlElement(&doc, gameElement, "note", std::to_string(game.rating));
            Api::addXmlElement(&doc, gameElement, "rotation", std::to_string(game.rotation));
            Api::addXmlElement(&doc, gameElement, "resolution", game.resolution);
            //Api::addXmlElement(&doc, gameElement, "controles", game.inputs);
            //Api::addXmlElement(&doc, gameElement, "couleurs", game.colors);

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
    gameList.topStaffs = topStaffs;
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
                            && (system == "ALL" || game.system.text == system)
                            && (editor == "ALL" || game.editor.text == editor)
                            && (developer == "ALL" || game.developer.text == developer)
                            && (player == "ALL" || game.players == player)
                            && (rating == "ALL" || game.rating == Api::parseInt(rating))
                            && (topstaff == "ALL" || game.topStaff == Api::parseBool(topstaff))
                            && (rotation == "ALL" || game.rotation == Api::parseInt(rotation))
                            && (resolution == "ALL" || game.resolution == resolution)
                            && (date == "ALL" || game.getDate(Game::Country::WOR).text == date)
                            && (genre == "ALL" || game.getGenre(Game::Language::EN).text == genre);
                 });

    return gameList;
}

std::vector<Game> GameList::findByName(const std::string &name) {

    std::vector<Game> matches;

    auto it = std::copy_if(games.begin(), games.end(), std::back_inserter(matches), [name](const Game &game) {
        return game.getName().text == name;
    });

    return matches;
}

std::vector<Game> GameList::findByName(const Game &game) {

    std::vector<Game> matches;

    auto it = std::copy_if(games.begin(), games.end(), std::back_inserter(matches), [game](const Game &g) {
        return game.getName().text == g.getName().text && game.path != g.path;
    });

    return matches;
}

Game GameList::findByRomId(long romId) {

    auto it = std::find_if(games.begin(), games.end(), [romId](const Game &game) {
        return game.romId == romId;
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

Game GameList::findByPathAndSystem(const std::string &path, int systemId) {

    auto it = std::find_if(games.begin(), games.end(), [path, systemId](const Game &game) {
        return game.system.id == systemId && game.path == path;
    });

    if (it != games.end()) {
        return *it;
    }

    return Game();
}

bool GameList::exist(long romId) {

    auto it = std::find_if(games.begin(), games.end(), [romId](const Game &game) {
        return game.romId == romId;
    });

    return it != games.end();
}

bool GameList::remove(long romId) {

    auto it = std::find_if(games.begin(), games.end(), [romId](const Game &game) {
        return game.romId == romId;
    });

    if (it != games.end()) {
        games.erase(it);
        return true;
    }

    return false;
}

size_t GameList::getAvailableCount() {
    return std::count_if(games.begin(), games.end(), [](const Game &game) {
        return game.available;
    });
}
