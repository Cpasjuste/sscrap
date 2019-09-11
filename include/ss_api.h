//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_API_H
#define SS_API_H

#include <json-c/json.h>

#include "ss_curl.h"
#include "ss_jeu.h"
#include "ss_user.h"

#define SS_TIMEOUT 10

namespace ss_api {

    class Api {

    public:

        enum Country {
            ALL, DE, ASI, AU, BR, BG, CA, CL,
            CN, AME, KR, CUS, DK, SP, EU,
            FI, FR, GR, HU, IL, IT, JP,
            KW, WOR, MOR, NO, NZ, OCE,
            NL, PE, PL, PT, CZ, UK, RU,
            SS, SK, SE, TW, TR, US
        };

        class GameSearch {
        public:
            User ssuser;
            std::vector<Game> games;
            std::string json;

            bool save(const std::string &dstPath);
        };

        class GameInfo {
        public:
            User ssuser;
            Game game;
            std::string json;

            bool save(const std::string &dstPath);
        };

        explicit Api(const std::string &devid, const std::string &devpassword,
                     const std::string &softname);

        GameSearch gameSearch(const std::string &recherche, const std::string &systemeid,
                              const std::string &ssid = "", const std::string &sspassword = "");

        GameSearch gameSearch(const std::string &srcPath);

        GameInfo gameInfo(const std::string &crc, const std::string &md5, const std::string &sha1,
                          const std::string &systemeid, const std::string &romtype,
                          const std::string &romnom, const std::string &romtaille, const std::string &gameid,
                          const std::string &ssid = "", const std::string &sspassword = "");

        GameInfo gameInfo(const std::string &srcPath);

        int download(const Game::Media &media, const std::string &dstPath);

        static std::vector<Game::Media> getMedia(
                const Game &game, const Game::Media::Type &type, const Country &country = ALL);

        static std::string mediaTypeToString(const Game::Media::Type &type);

        static std::string countryToString(const Country &country);

    private:

        static GameInfo parseGameInfo(const std::string &jsonData);

        static GameSearch parseGameSearch(const std::string &jsonData);

        static Game parseGame(json_object *root);

        static User parseUser(json_object *root);

        static json_object *getJsonObject(json_object *root, const std::string &key);

        static std::string getJsonString(json_object *root, const std::string &key);

        std::string devid;
        std::string devpassword;
        std::string softname;

        Curl curl;
    };
}

#endif //SCREENSCRAP_SS_API_H
