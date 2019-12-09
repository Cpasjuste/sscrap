//
// Created by cpasjuste on 03/12/2019.
//

#ifndef SSCRAP_SS_GAMELIST_H
#define SSCRAP_SS_GAMELIST_H

#include <string>
#include "ss_game.h"

namespace ss_api {

    class GameList {
    public:

        enum Format {
            ScreenScrapper,
            EmulationStation,
            FbNeo,
            Unknown
        };

        GameList() = default;

        explicit GameList(const std::string &xmlPath, const std::string &romPath = "");

        bool append(const std::string &xmlPath, const std::string &romPath = "");

        GameList filter(bool available = false, bool clones = false,
                        const std::string &system = "All", const std::string &editor = "All",
                        const std::string &developer = "All", const std::string &player = "All",
                        const std::string &rating = "All", const std::string &topstaff = "All",
                        const std::string &rotation = "All", const std::string &resolution = "All",
                        const std::string &date = "All", const std::string &genre = "All");

        bool save(const std::string &dstPath,
                  const Game::Language &language = Game::Language::EN,
                  const Format &format = ScreenScrapper);

        Game findByRomId(const std::string &romId);

        Game findByPath(const std::string &path);

        bool exist(const std::string &romId);

        bool remove(const std::string &romId);

        bool fixClones(const std::string &fbnDatPath);

        int getAvailableCount();

        std::string xml;
        Format format = EmulationStation;
        std::vector<std::string> romPaths;
        std::vector<Game> games;
        std::vector<std::string> systems;
        std::vector<std::string> editors;
        std::vector<std::string> developers;
        std::vector<std::string> players;
        std::vector<std::string> ratings;
        std::vector<std::string> topstaffs;
        std::vector<std::string> rotations;
        std::vector<std::string> resolutions;
        std::vector<std::string> dates;
        std::vector<std::string> genres;
    };
}

#endif //SSCRAP_SS_GAMELIST_H
