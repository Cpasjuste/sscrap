//
// Created by cpasjuste on 11/09/2019.
//

#include <algorithm>
#include <ss_game.h>

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

Game::Media Game::getMedia(const Game::Media::Type &type, const Game::Country &country) const {

    std::vector<Game::Media> mediaList;

    remove_copy_if(medias.begin(), medias.end(), back_inserter(mediaList),
                   [type, country](const Game::Media &media) {
                       return media.type != Api::toString(type)
                              || (country != Country::ALL && media.country != Api::toString(country));
                   });

    if (mediaList.empty()) {
        if (country != Game::Country::SS) {
            return getMedia(type, Game::Country::SS);
        }
        return Game::Media();
    }
    return mediaList.at(0);
}

int Game::Media::download(const std::string &dstPath) {

    if (dstPath.empty()) {
        return -1;
    }

    //printf("Game::Media::download: %s\n", url.c_str());
    long http_code = 0;
    Curl ss_curl;
    int res = ss_curl.getData(url, dstPath, SS_TIMEOUT, &http_code);
    if (res != 0) {
        printf("Game::Media::download: error: curl failed: %s, http_code: %li\n",
               curl_easy_strerror((CURLcode) res), http_code);
        return (int) http_code;
    }

    return 0;
}
