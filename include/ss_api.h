//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_API_H
#define SS_API_H

#include <tinyxml2.h>

#include "ss_curl.h"
#include "ss_game.h"
#include "ss_user.h"

#define SS_TIMEOUT 10

namespace ss_api {

    class Api {

    public:

        class GameList {
        public:
            std::vector<Game> games;
            std::string xml;
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

            bool save(const std::string &dstPath);
        };

        class GameSearch {
        public:
            User ssuser;
            std::vector<Game> games;
            std::string xml;
        };

        class GameInfo {
        public:
            User ssuser;
            Game game;
            std::string xml;
        };

        static GameSearch gameSearch(const std::string &recherche, const std::string &systemeid,
                                     const std::string &ssid = "", const std::string &sspassword = "");

        static GameInfo gameInfo(const std::string &crc, const std::string &md5, const std::string &sha1,
                                 const std::string &systemeid, const std::string &romtype,
                                 const std::string &romnom, const std::string &romtaille, const std::string &gameid,
                                 const std::string &ssid = "", const std::string &sspassword = "");

        static GameList gameList(const std::string &xmlPath, const std::string &romPath = "");

        static std::vector<Game> gameListFilter(const std::vector<Game> &games,
                                                const std::string &system = "All",
                                                const std::string &editor = "All",
                                                const std::string &developer = "All",
                                                const std::string &player = "All",
                                                const std::string &rating = "All",
                                                const std::string &topstaff = "All",
                                                const std::string &rotation = "All",
                                                const std::string &resolution = "All",
                                                const std::string &date = "All",
                                                const std::string &genre = "All");

        static std::string toString(const Game::Media::Type &type);

        static std::string toString(const Game::Country &country);

        static std::string toString(const Game::Language &language);

        static Game::Media::Type toMedia(const std::string &type);

        static Game::Country toCountry(const std::string &country);

        static Game::Language toLanguage(const std::string &language);

        static std::string ss_devid;
        static std::string ss_devpassword;
        static std::string ss_softname;

    private:

        static GameInfo parseGameInfo(const std::string &xmlData, const std::string &romName);

        static GameSearch parseGameSearch(const std::string &xmlData);

        static Game parseGame(tinyxml2::XMLNode *gameNode, const std::string &romName = "");

        static User parseUser(tinyxml2::XMLNode *userNode);

        static std::string getXmlAttribute(tinyxml2::XMLElement *element, const std::string &name);

        static std::string getXmlText(tinyxml2::XMLElement *element);

        static bool sortByName(const std::string &g1, const std::string &g2);
    };
}

#endif //SS_API_H
