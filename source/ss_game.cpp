//
// Created by cpasjuste on 11/09/2019.
//

#include <algorithm>
#include "ss_api.h"

using namespace ss_api;

Game::Name Game::getName(const Game::Country &country) const {

    std::vector<Game::Name> list;

    remove_copy_if(names.begin(), names.end(), back_inserter(list),
                   [country](const Game::Name &name) {
                       return country != Country::ALL && name.country != country;
                   });

    if (list.empty()) {
        if (country != Game::Country::SS) {
            return getName(Game::Country::SS);
        } else if (!names.empty()) {
            return names.at(0);
        }
        return Game::Name();
    } else {
        return list.at(0);
    }
}

Game::Synopsis Game::getSynopsis(const Game::Language &language) const {

    std::vector<Game::Synopsis> list;

    copy_if(synopses.begin(), synopses.end(), back_inserter(list),
            [language](const Game::Synopsis &p) {
                return p.language == language;
            });

    if (list.empty()) {
        if (language != Game::Language::EN) {
            return getSynopsis(Game::Language::EN);
        } else if (!synopses.empty()) {
            return synopses.at(0);
        }
        return Game::Synopsis();
    }

    return list.at(0);
}

Game::Date Game::getDate(const Game::Country &country) const {

    std::vector<Game::Date> list;

    copy_if(dates.begin(), dates.end(), back_inserter(list),
            [country](const Game::Date &p) {
                return p.country == country;
            });

    if (list.empty()) {
        if (country != Game::Country::SS) {
            return getDate(Game::Country::SS);
        } else if (!dates.empty()) {
            return dates.at(0);
        }
        return Game::Date();
    }

    return list.at(0);
}

Game::Genre Game::getGenre(const Game::Language &language) const {

    std::vector<Game::Genre> list;

    copy_if(genres.begin(), genres.end(), back_inserter(list),
            [language](const Game::Genre &p) {
                return p.language == language;
            });

    if (list.empty()) {
        if (language != Game::Language::EN) {
            return getGenre(Game::Language::EN);
        } else if (!genres.empty()) {
            return genres.at(0);
        }
        return Game::Genre();
    }

    return list.at(0);
}

Game::Media Game::getMedia(const std::string &mediaTypeName, const Game::Country &country) const {

    std::vector<Game::Media> mediaList;

    remove_copy_if(medias.begin(), medias.end(), back_inserter(mediaList),
                   [mediaTypeName, country](const Game::Media &media) {
                       return media.type != mediaTypeName
                              || (country != Country::ALL && media.country != country);
                   });

    if (mediaList.empty()) {
        if (country != Game::Country::WOR && country != Game::Country::ALL) {
            return getMedia(mediaTypeName, Game::Country::WOR);
        } else if (country != Game::Country::ALL) {
            return getMedia(mediaTypeName, Game::Country::ALL);
        }
        return Game::Media();
    }

    return mediaList.at(0);
}

bool Game::isClone() const {
    return !cloneOf.empty() && cloneOf != "0";
}

int Game::Media::download(const std::string &dstPath, int retryDelay) {

    if (dstPath.empty()) {
        return -1;
    }

    SS_PRINT("Game::Media::download: %s\n", url.c_str());
    long code = 0;
    Curl ss_curl;
    int res = ss_curl.getData(url, dstPath, SS_TIMEOUT, &code);
    if (retryDelay > 0) {
        while (code == 429 || code == 28) {
            Api::printe((int) code, retryDelay);
            Io::delay(retryDelay);
            res = ss_curl.getData(url, dstPath, SS_TIMEOUT, &code);
        }
    }

    if (res != 0) {
        SS_PRINT("Game::Media::download: error: curl failed: %s, http_code: %li\n",
                 curl_easy_strerror((CURLcode) res), code);
        return (int) code;
    }

    return 0;
}

bool Game::parseGame(Game *game, tinyxml2::XMLNode *gameNode, const std::string &romName, const int &GameListFormat) {

    tinyxml2::XMLElement *element = nullptr;

    if (game == nullptr || gameNode == nullptr) {
        return false;
    }

    if (GameListFormat == GameList::Format::FbNeo) {
        game->names.emplace_back(Country::WOR, Api::getXmlTextStr(gameNode->FirstChildElement("description")));
        game->path = Api::getXmlAttrStr(gameNode->ToElement(), "name");
        if (!game->path.empty()) {
            game->path += ".zip";
        }
        game->dates.emplace_back(Country::WOR, Api::getXmlTextStr(gameNode->FirstChildElement("year")));
        game->developer.text = Api::getXmlTextStr(gameNode->FirstChildElement("manufacturer"));
        game->editor.text = game->developer.text;
        game->cloneOf = Api::getXmlAttrStr(gameNode->ToElement(), "cloneof");
        return true;
    }

    // screenscraper / emulationstation compat
    game->id = Api::getXmlAttrInt(gameNode->ToElement(), "id");
    // screenscraper
    game->romId = Api::getXmlAttrInt(gameNode->ToElement(), "romid");
    // screenscraper
    game->notGame = Api::getXmlAttrBool(gameNode->ToElement(), "notgame");
    // emulationstation compat
    game->source = Api::getXmlAttrStr(gameNode->ToElement(), "source");
    // emulationstation compat
    game->path = Api::getXmlTextStr(gameNode->FirstChildElement("path"));
    if (game->path.empty()) {
        game->path = romName;
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("noms");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("nom");
        while (node != nullptr) {
            game->names.emplace_back(Api::getXmlAttrCountry(node->ToElement(), "region"),
                                     Api::getXmlTextStr(node->ToElement()));
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->names.emplace_back(Country::WOR,
                                 Api::getXmlTextStr(gameNode->FirstChildElement("name")));
    }
    // screenscraper
    element = gameNode->FirstChildElement("regions");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("region");
        while (node != nullptr) {
            game->countries.emplace_back(Api::getXmlTextCountry(node->ToElement()));
            node = node->NextSibling();
        }
    }
    // screenscraper
    game->cloneOf = Api::getXmlTextStr(gameNode->FirstChildElement("cloneof"));
    // screenscraper
    game->system.id = Api::getXmlAttrInt(gameNode->FirstChildElement("systeme"), "id");
    game->system.parentId = Api::getXmlAttrInt(gameNode->FirstChildElement("systeme"), "parentid");
    game->system.text = Api::getXmlTextStr(gameNode->FirstChildElement("systeme"));
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("synopsis");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("synopsis");
        while (node != nullptr) {
            Game::Synopsis synopsis{};
            synopsis.language = Api::getXmlAttrLang(node->ToElement(), "langue");
            synopsis.text = Api::getXmlTextStr(node->ToElement());
            game->synopses.emplace_back(synopsis);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->synopses.emplace_back(Language::EN, Api::getXmlTextStr(gameNode->FirstChildElement("desc")));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("medias");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("media");
        while (node != nullptr) {
            Game::Media media{};
            media.parent = Api::getXmlAttrStr(node->ToElement(), "parent");
            media.type = Api::getXmlAttrStr(node->ToElement(), "type");
            media.crc = Api::getXmlAttrStr(node->ToElement(), "crc");
            media.md5 = Api::getXmlAttrStr(node->ToElement(), "md5");
            media.sha1 = Api::getXmlAttrStr(node->ToElement(), "sha1");
            media.format = Api::getXmlAttrStr(node->ToElement(), "format");
            media.support = Api::getXmlAttrStr(node->ToElement(), "support");
            media.url = Api::getXmlTextStr(node->ToElement());
            media.country = Api::getXmlAttrCountry(node->ToElement(), "region");
            game->medias.emplace_back(media);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat
        game->medias.push_back({"sstitle", "",
                                Api::getXmlTextStr(gameNode->FirstChildElement("image")),
                                "", "", "", "", "", Country::WOR});
        game->medias.push_back({"ss", "",
                                Api::getXmlTextStr(gameNode->FirstChildElement("thumbnail")),
                                "", "", "", "", "", Country::WOR});
        game->medias.push_back({"video", "",
                                Api::getXmlTextStr(gameNode->FirstChildElement("video")),
                                "", "", "", "", "", Country::WOR});
    }
    // screenscraper (prioritise screenscraper format)
    game->rating = Api::getXmlTextInt(gameNode->FirstChildElement("note"));
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("dates");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("date");
        while (node != nullptr) {
            Game::Date date{};
            date.country = Api::getXmlAttrCountry(node->ToElement(), "region");
            date.text = Api::getXmlTextStr(node->ToElement());
            if (date.text.size() >= 4) {
                date.text = date.text.substr(0, 4);
            }
            game->dates.emplace_back(date);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->dates.emplace_back(Country::WOR, Api::getXmlTextStr(gameNode->FirstChildElement("releasedate")));
    }
    // screenscraper
    game->developer.id = Api::getXmlAttrInt(gameNode->FirstChildElement("developpeur"), "id");
    game->developer.text = Api::getXmlTextStr(gameNode->FirstChildElement("developpeur"));
    if (game->developer.text.empty()) {
        // emulationstation compat (use emulationstation format)
        game->developer.text = Api::getXmlTextStr(gameNode->FirstChildElement("developer"));
    }
    // screenscraper
    game->editor.id = Api::getXmlAttrInt(gameNode->FirstChildElement("editeur"), "id");
    game->editor.text = Api::getXmlTextStr(gameNode->FirstChildElement("editeur"));
    if (game->editor.text.empty()) {
        // emulationstation compat (use emulationstation format)
        game->editor.text = Api::getXmlTextStr(gameNode->FirstChildElement("publisher"));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("genres");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("genre");
        while (node != nullptr) {
            Game::Genre genre{};
            genre.id = Api::getXmlAttrInt(node->ToElement(), "id");
            genre.mainId = Api::getXmlAttrInt(node->ToElement(), "principale");
            genre.parentId = Api::getXmlAttrInt(node->ToElement(), "parentid");
            genre.language = Api::getXmlAttrLang(node->ToElement(), "langue");
            genre.text = Api::getXmlTextStr(node->ToElement());
            game->genres.emplace_back(genre);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->genres.emplace_back(0, 0, 0,
                                  Api::getXmlTextStr(gameNode->FirstChildElement("genre")),
                                  Language::EN);
    }
    // screenscraper
    game->players = Api::getXmlTextStr(gameNode->FirstChildElement("joueurs"));
    if (game->players.empty()) {
        // emulationstation compat (use emulationstation format)
        game->players = Api::getXmlTextStr(gameNode->FirstChildElement("players"));
    }
    // screenscraper
    game->topStaff = Api::getXmlTextBool(gameNode->FirstChildElement("topstaff"));
    // screenscraper
    game->rotation = Api::getXmlTextInt(gameNode->FirstChildElement("rotation"));
    // screenscraper
    game->resolution = Api::getXmlTextStr(gameNode->FirstChildElement("resolution"));
    // screenscraper
    game->inputs = Api::getXmlTextStr(gameNode->FirstChildElement("controles"));
    // screenscraper
    game->colors = Api::getXmlTextStr(gameNode->FirstChildElement("couleurs"));

    return true;
}
