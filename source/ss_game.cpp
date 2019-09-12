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

Game::Genre::Name Game::getGenre(const Game::Language &language) const {

    for (const auto &genre : genres) {
        for (const auto &name : genre.names) {
            if (name.language == Api::toString(language)) {
                return name;
            }
        }
    }

    if (language != Game::Language::EN) {
        return getGenre(Game::Language::EN);
    } else if (!genres.empty() && !genres.at(0).names.empty()) {
        return genres.at(0).names.at(0);
    }

    return Game::Genre::Name();
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
