//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_API_H
#define SS_API_H

#include <tinyxml2.h>

#include "ss_curl.h"
#include "ss_game.h"
#include "ss_user.h"

#define KRED "\x1B[91m"
#define KGRE "\x1B[92m"
#define KYEL "\x1B[93m"
#define KRAS "\033[0m"

#define SS_TIMEOUT 10

namespace ss_api {

    class Api {

    public:

        class GameSearch {
        public:
            GameSearch() = default;

            explicit GameSearch(int err) {
                http_error = err;
            };
            User ssuser;
            std::vector<Game> games;
            std::string xml;
            int http_error = 0;
        };

        class GameInfo {
        public:
            GameInfo() = default;

            explicit GameInfo(int err) {
                http_error = err;
            };
            User ssuser;
            Game game;
            std::string xml;
            int http_error = 0;
        };

        static GameSearch gameSearch(const std::string &recherche, const std::string &systemeid,
                                     const std::string &ssid = "", const std::string &sspassword = "");

        static GameInfo gameInfo(const std::string &crc, const std::string &md5, const std::string &sha1,
                                 const std::string &systemeid, const std::string &romtype,
                                 const std::string &romnom, const std::string &romtaille, const std::string &gameid,
                                 const std::string &ssid = "", const std::string &sspassword = "");

        static std::string toString(const Game::Media::Type &type);

        static std::string toString(const Game::Country &country);

        static std::string toString(const Game::Language &language);

        static Game::Media::Type toMedia(const std::string &type);

        static Game::Country toCountry(const std::string &country);

        static Game::Language toLanguage(const std::string &language);

        static std::string ss_devid;
        static std::string ss_devpassword;
        static std::string ss_softname;

        // internal
        static Game parseGame(tinyxml2::XMLNode *gameNode, const std::string &romName = "");

        static bool sortByName(const std::string &g1, const std::string &g2);

        static bool sortGameByName(const Game &g1, const Game &g2);

        static std::string getXmlAttribute(tinyxml2::XMLElement *element, const std::string &name);

        static std::string getXmlText(tinyxml2::XMLElement *element);

    private:

        static GameInfo parseGameInfo(const std::string &xmlData, const std::string &romName);

        static GameSearch parseGameSearch(const std::string &xmlData);

        static User parseUser(tinyxml2::XMLNode *userNode);
    };
}

// ss_debug print
extern bool ss_debug;
#define SS_PRINT(f_, ...) \
do \
{ \
    if(ss_debug) \
        printf((f_), ##__VA_ARGS__); \
} \
while(0)

#define SS_PRINT_RED(f_, ...) \
do \
{ \
    if(ss_debug) { \
        printf(KRED "err: "); \
        printf((f_), ##__VA_ARGS__); \
        printf(KRAS); \
    } \
} \
while(0)

#endif //SS_API_H
