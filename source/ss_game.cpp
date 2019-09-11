//
// Created by cpasjuste on 11/09/2019.
//

#include <algorithm>
#include "ss_api.h"

using namespace ss_api;

Game::Media Game::getMedia(const Game::Media::Type &type, const Game::Country &country) const {

    std::vector<Game::Media> mediaList;

    remove_copy_if(medias.begin(), medias.end(), back_inserter(mediaList),
                   [type, country](const Game::Media &media) {
                       return media.type != Api::mediaTypeToString(type)
                              || (country != Country::ALL && media.country != Api::countryToString(country));
                   });

    if (mediaList.empty()) {
        return Game::Media();
    } else {
        return mediaList.at(0);
    }
}

std::vector<Game::Media> Game::getMedias(const Game::Media::Type &type, const Game::Country &country) const {

    std::vector<Game::Media> mediaList;

    remove_copy_if(medias.begin(), medias.end(), back_inserter(mediaList),
                   [type, country](const Game::Media &media) {
                       return media.type != Api::mediaTypeToString(type)
                              || (country != Country::ALL && media.country != Api::countryToString(country));
                   });

    return mediaList;
}
