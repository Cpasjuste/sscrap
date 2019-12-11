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
                       return country != Country::ALL && name.country != Api::toString(country);
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
                return p.language == Api::toString(language);
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
                return p.country == Api::toString(country);
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
                return p.language == Api::toString(language);
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
                              || (country != Country::ALL && media.country != Api::toString(country));
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
    return !cloneof.empty() && cloneof != "0";
}

int Game::Media::download(const std::string &dstPath) {

    if (dstPath.empty()) {
        return -1;
    }

    SS_PRINT("Game::Media::download: %s\n", url.c_str());
    long http_code = 0;
    Curl ss_curl;
    int res = ss_curl.getData(url, dstPath, SS_TIMEOUT, &http_code);
    if (res != 0) {
        SS_PRINT("Game::Media::download: error: curl failed: %s, http_code: %li\n",
                 curl_easy_strerror((CURLcode) res), http_code);
        return (int) http_code;
    }

    return 0;
}

bool Game::parseGame(Game *game, tinyxml2::XMLNode *gameNode, const std::string &romName, const int &GameListFormat) {

    tinyxml2::XMLElement *element = nullptr;

    if (game == nullptr || gameNode == nullptr) {
        return false;
    }

    if (GameListFormat == GameList::Format::FbNeo) {
        game->names.emplace_back("wor", Api::getXmlText(gameNode->FirstChildElement("description")));
        game->path = Api::Api::getXmlAttribute(gameNode->ToElement(), "name");
        if (!game->path.empty()) {
            game->path += ".zip";
        }
        game->dates.emplace_back("wor", Api::getXmlText(gameNode->FirstChildElement("year")));
        game->developer.text = Api::getXmlText(gameNode->FirstChildElement("manufacturer"));
        game->editor.text = game->developer.text;
        game->cloneof = Api::Api::getXmlAttribute(gameNode->ToElement(), "cloneof");
        return true;
    }

    // screenscraper / emulationstation compat
    game->id = Api::getXmlAttribute(gameNode->ToElement(), "id");
    // screenscraper
    game->romid = Api::getXmlAttribute(gameNode->ToElement(), "romid");
    // screenscraper
    game->notgame = Api::getXmlAttribute(gameNode->ToElement(), "notgame");
    // emulationstation compat
    game->source = Api::getXmlAttribute(gameNode->ToElement(), "source");
    // emulationstation compat
    game->path = Api::getXmlText(gameNode->FirstChildElement("path"));
    if (game->path.empty()) {
        game->path = romName;
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("noms");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("nom");
        while (node != nullptr) {
            game->names.emplace_back(Api::getXmlAttribute(node->ToElement(), "region"),
                                     Api::getXmlText(node->ToElement()));
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->names.emplace_back(Api::toString(Game::Country::WOR),
                                 Api::getXmlText(gameNode->FirstChildElement("name")));
    }
    // screenscraper
    element = gameNode->FirstChildElement("regions");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("region");
        while (node != nullptr) {
            game->countries.emplace_back(Api::getXmlText(node->ToElement()));
            node = node->NextSibling();
        }
    }
    // screenscraper
    game->cloneof = Api::getXmlText(gameNode->FirstChildElement("cloneof"));
    // screenscraper
    game->system.id = Api::getXmlAttribute(gameNode->FirstChildElement("systeme"), "id");
    game->system.text = Api::getXmlText(gameNode->FirstChildElement("systeme"));
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("synopsis");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("synopsis");
        while (node != nullptr) {
            Game::Synopsis synopsis{};
            synopsis.language = Api::getXmlAttribute(node->ToElement(), "langue");
            synopsis.text = Api::getXmlText(node->ToElement());
            game->synopses.emplace_back(synopsis);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->synopses.emplace_back("wor", Api::getXmlText(gameNode->FirstChildElement("desc")));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("medias");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("media");
        while (node != nullptr) {
            Game::Media media{};
            media.parent = Api::getXmlAttribute(node->ToElement(), "parent");
            media.type = Api::getXmlAttribute(node->ToElement(), "type");
            media.crc = Api::getXmlAttribute(node->ToElement(), "crc");
            media.md5 = Api::getXmlAttribute(node->ToElement(), "md5");
            media.sha1 = Api::getXmlAttribute(node->ToElement(), "sha1");
            media.format = Api::getXmlAttribute(node->ToElement(), "format");
            media.support = Api::getXmlAttribute(node->ToElement(), "support");
            media.url = Api::getXmlText(node->ToElement());
            media.country = Api::getXmlAttribute(node->ToElement(), "region");
            if (media.country.empty()) {
                media.country = Api::toString(Game::Country::UNKNOWN);
            }
            game->medias.emplace_back(media);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat
        game->medias.push_back({"sstitle", "",
                                Api::getXmlText(gameNode->FirstChildElement("image")), "wor", "", "", "", "", ""});
        game->medias.push_back({"ss", "",
                                Api::getXmlText(gameNode->FirstChildElement("thumbnail")), "wor", "", "", "", "", ""});
        game->medias.push_back({"video", "",
                                Api::getXmlText(gameNode->FirstChildElement("video")), "wor", "", "", "", "", ""});
    }
    // screenscraper (prioritise screenscraper format)
    game->rating = Api::getXmlText(gameNode->FirstChildElement("note"));
    if (game->rating.empty()) {
        // emulationstation compat (use emulationstation format)
        game->rating = Api::getXmlText(gameNode->FirstChildElement("rating"));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("dates");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("date");
        while (node != nullptr) {
            Game::Date date{};
            date.country = Api::getXmlAttribute(node->ToElement(), "region");
            date.text = Api::getXmlText(node->ToElement());
            if (date.text.size() >= 4) {
                date.text = date.text.substr(0, 4);
            }
            game->dates.emplace_back(date);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->dates.emplace_back("wor", Api::getXmlText(gameNode->FirstChildElement("releasedate")));
    }
    // screenscraper
    game->developer.id = Api::getXmlAttribute(gameNode->FirstChildElement("developpeur"), "id");
    game->developer.text = Api::getXmlText(gameNode->FirstChildElement("developpeur"));
    if (game->developer.text.empty()) {
        // emulationstation compat (use emulationstation format)
        game->developer.text = Api::getXmlText(gameNode->FirstChildElement("developer"));
    }
    // screenscraper
    game->editor.id = Api::getXmlAttribute(gameNode->FirstChildElement("editeur"), "id");
    game->editor.text = Api::getXmlText(gameNode->FirstChildElement("editeur"));
    if (game->editor.text.empty()) {
        // emulationstation compat (use emulationstation format)
        game->editor.text = Api::getXmlText(gameNode->FirstChildElement("publisher"));
    }
    // screenscraper (prioritise screenscraper format)
    element = gameNode->FirstChildElement("genres");
    if (element != nullptr) {
        tinyxml2::XMLNode *node = element->FirstChildElement("genre");
        while (node != nullptr) {
            Game::Genre genre{};
            genre.id = Api::getXmlAttribute(node->ToElement(), "id");
            genre.main = Api::getXmlAttribute(node->ToElement(), "principale");
            genre.parentid = Api::getXmlAttribute(node->ToElement(), "parentid");
            genre.language = Api::getXmlAttribute(node->ToElement(), "langue");
            genre.text = Api::getXmlText(node->ToElement());
            game->genres.emplace_back(genre);
            node = node->NextSibling();
        }
    } else {
        // emulationstation compat (use emulationstation format)
        game->genres.emplace_back(
                "", "", "", "en", Api::getXmlText(gameNode->FirstChildElement("genre")));
    }
    // screenscraper
    game->players = Api::getXmlText(gameNode->FirstChildElement("joueurs"));
    if (game->players.empty()) {
        // emulationstation compat (use emulationstation format)
        game->players = Api::getXmlText(gameNode->FirstChildElement("players"));
    }
    // screenscraper
    game->topstaff = Api::getXmlText(gameNode->FirstChildElement("topstaff"));
    // screenscraper
    game->rotation = Api::getXmlText(gameNode->FirstChildElement("rotation"));
    // screenscraper
    game->resolution = Api::getXmlText(gameNode->FirstChildElement("resolution"));
    // screenscraper
    game->inputs = Api::getXmlText(gameNode->FirstChildElement("controles"));
    // screenscraper
    game->colors = Api::getXmlText(gameNode->FirstChildElement("couleurs"));

    return true;
}
