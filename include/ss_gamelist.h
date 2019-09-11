//
// Created by cpasjuste on 11/09/2019.
//

#ifndef SSSCRAP_SS_GAMELIST_H
#define SSSCRAP_SS_GAMELIST_H

#include <string>
#include <vector>
#include <tinyxml2.h>
#include "ss_game.h"

namespace ss_api {

    class GameList {

    public:

        explicit GameList() = default;

        bool load(const std::string &xmlPath);

        bool save(const std::string &xmlPath);


        void clear();

        std::vector<Game> filter(const std::string &date = "All", const std::string &developer = "All",
                                 const std::string &publisher = "All", const std::string &genre = "All",
                                 const std::string &players = "All");

        std::vector<Game> games;
        std::vector<std::string> releasedateList;
        std::vector<std::string> developerList;
        std::vector<std::string> editorList;
        std::vector<std::string> genreList;
        std::vector<std::string> playersList;
        std::vector<std::string> romsPaths;

    private:

        static bool sortByName(const std::string &g1, const std::string &g2);
    };
}

#endif //SSSCRAP_SS_GAMELIST_H
