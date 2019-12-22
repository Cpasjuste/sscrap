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
#include "ss_systemlist.h"

#define SS_TIMEOUT 10

#ifdef _MSC_VER
#define COLOR_R (FOREGROUND_INTENSITY | FOREGROUND_RED)
#define COLOR_G (FOREGROUND_INTENSITY | FOREGROUND_GREEN)
#define COLOR_Y (FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN)
#else
#define COLOR_R "\x1B[91m"
#define COLOR_G "\x1B[92m"
#define COLOR_Y "\x1B[93m"
#define COLOR_O "\x1B[33m"
#endif

namespace ss_api {

    class Api {

    public:

        static std::string toString(const Game::Country &country);

        static std::string toString(const Game::Language &language);

        static Game::Country toCountry(const std::string &country);

        static Game::Language toLanguage(const std::string &language);

        static int parseInt(const std::string &str, int defValue = 0);

        static bool parseBool(const std::string &str, bool defValue = false);

        // internal
        static bool sortByName(const std::string &g1, const std::string &g2);

        static bool sortGameByName(const Game &g1, const Game &g2);

        // get attributes
        static std::string getXmlAttrStr(tinyxml2::XMLElement *element, const std::string &name);

        static int getXmlAttrInt(tinyxml2::XMLElement *element, const std::string &name);

        static bool getXmlAttrBool(tinyxml2::XMLElement *element, const std::string &name);

        static Game::Country getXmlAttrCountry(tinyxml2::XMLElement *element, const std::string &name);

        static Game::Language getXmlAttrLang(tinyxml2::XMLElement *element, const std::string &name);

        // get text
        static std::string getXmlTextStr(tinyxml2::XMLElement *element);

        static int getXmlTextInt(tinyxml2::XMLElement *element);

        static bool getXmlTextBool(tinyxml2::XMLElement *element);

        static Game::Country getXmlTextCountry(tinyxml2::XMLElement *element);

        static Game::Language getXmlTextLang(tinyxml2::XMLElement *element);

        // add
        static tinyxml2::XMLElement *addXmlElement(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *parent,
                                                   const std::string &name, const std::string &value);

#ifdef _MSC_VER
        static void printc(int color, const char* format, ...);
#else
        static void printc(const char *color, const char *format, ...);
#endif
        static void printe(int code, int delay);

        static std::string ss_devid;
        static std::string ss_devpassword;
        static std::string ss_softname;
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

#endif //SS_API_H
