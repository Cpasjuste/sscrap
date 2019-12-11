//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_API_H
#define SS_API_H

#include <tinyxml2.h>

#include "ss_curl.h"
#include "ss_io.h"
#include "ss_game.h"
#include "ss_user.h"
#include "ss_gameinfo.h"
#include "ss_gamesearch.h"
#include "ss_gamelist.h"
#include "ss_mediasgamelist.h"

#define KRED "\x1B[91m"
#define KGRE "\x1B[92m"
#define KYEL "\x1B[93m"
#define KRAS "\033[0m"

#define SS_TIMEOUT 10

namespace ss_api {

    class Api {

    public:

        static std::string toString(const Game::Country &country);

        static std::string toString(const Game::Language &language);

        static Game::Country toCountry(const std::string &country);

        static Game::Language toLanguage(const std::string &language);

        static int parseInt(const std::string &str, int defValue = 0);

        static void printError(int code, int delay);

        static std::string ss_devid;
        static std::string ss_devpassword;
        static std::string ss_softname;

        // internal
        static bool sortByName(const std::string &g1, const std::string &g2);

        static bool sortGameByName(const Game &g1, const Game &g2);

        static std::string getXmlAttribute(tinyxml2::XMLElement *element, const std::string &name);

        static std::string getXmlText(tinyxml2::XMLElement *element);

        static tinyxml2::XMLElement *addXmlElement(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *parent,
                                                   const std::string &name, const std::string &value);
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
        printf(KRED "NOK: "); \
        printf((f_), ##__VA_ARGS__); \
        printf(KRAS); \
    } \
} \
while(0)

#endif //SS_API_H
