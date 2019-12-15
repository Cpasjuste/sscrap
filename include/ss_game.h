//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_GAME_H
#define SS_GAME_H

#include <string>
#include <vector>
#include <tinyxml2.h>

namespace ss_api {

    class Game {

    public:

        Game() = default;

        enum class Country {
            ALL, DE, ASI, AU, BR, BG, CA, CL,
            CN, AME, KR, CUS, DK, SP, EU,
            FI, FR, GR, HU, IL, IT, JP,
            KW, WOR, MOR, NO, NZ, OCE,
            NL, PE, PL, PT, CZ, UK, RU,
            SS, SK, SE, TW, TR, US,
            UNKNOWN
        };

        enum class Language {
            EN, FR, ES, PT,
            ALL, UNKNOWN
        };

        struct Name {
            Name() = default;

            Name(const std::string &c, const std::string &t) {
                country = c;
                text = t;
            }

            std::string country;
            std::string text;
        };

        struct System {
            System() = default;

            System(const std::string &sid, const std::string &pid, const std::string &t) {
                id = sid;
                parentId = pid;
                text = t;
            }

            std::string id;
            std::string parentId;
            std::string text;
        };

        struct Editor {
            Editor() = default;

            Editor(const std::string &c, const std::string &t) {
                id = c;
                text = t;
            }

            std::string id;
            std::string text;
        };

        struct Developer {
            Developer() = default;

            Developer(const std::string &c, const std::string &t) {
                id = c;
                text = t;
            }

            std::string id;
            std::string text;
        };

        struct Synopsis {
            Synopsis() = default;

            Synopsis(const std::string &l, const std::string &t) {
                language = l;
                text = t;
            }

            std::string language;
            std::string text;
        };

        struct Classification {
            Classification() = default;

            Classification(const std::string &ty, const std::string &tx) {
                type = ty;
                text = tx;
            }

            std::string type;
            std::string text;
        };

        struct Date {
            Date() = default;

            Date(const std::string &c, const std::string &t) {
                country = c;
                text = t;
            }

            std::string country;
            std::string text;
        };

        struct Genre {
            Genre() = default;

            Genre(const std::string &i, const std::string &m, const std::string &p,
                  const std::string &l, const std::string &t) {
                id = i;
                main = m;
                parentid = p;
                language = l;
                text = t;
            }

            std::string id;
            std::string main;
            std::string parentid;
            std::string language;
            std::string text;
        };

        struct Family {
            struct Name {
                std::string language;
                std::string text;
            };
            std::string id;
            std::string main;
            std::string parentid;
            std::vector<Family::Name> names;
        };

        struct Media {
            enum class Parent {
                All, Game, Country, Editor, Developer,
                Players, Rating, Genre
            };
            std::string type;
            std::string parent;
            std::string url;
            std::string country;
            std::string crc;
            std::string md5;
            std::string sha1;
            std::string format;
            std::string support;

            int download(const std::string &dstPath);
        };

        Game::Name getName(const Game::Country &country = Game::Country::WOR) const;

        Game::Synopsis getSynopsis(const Game::Language &language = Game::Language::EN) const;

        // TODO: to lazy for now
        //Game::Classification getClassification(const Game::Country &country) const;

        Game::Date getDate(const Game::Country &country = Game::Country::WOR) const;

        Game::Genre getGenre(const Game::Language &language = Game::Language::EN) const;

        // TODO: to lazy for now
        //Game::Family::Name getFamily(const Game::Language &language) const;

        Game::Media getMedia(const std::string &mediaTypeName, const Game::Country &country = Game::Country::WOR) const;

        bool isClone() const;

        static bool parseGame(Game *game, tinyxml2::XMLNode *gameNode,
                              const std::string &romName = "", const int &GameListFormat = 0);

        std::string id;
        std::string romid;
        std::string notgame;
        std::vector<Name> names;
        std::vector<std::string> countries;
        std::string cloneof;
        System system;
        Editor editor;
        Developer developer;
        std::string players;
        std::string rating;
        std::string topstaff;
        std::string rotation;
        std::string resolution;
        std::string inputs;
        std::string colors;
        std::vector<Synopsis> synopses;
        std::vector<Classification> classifications;
        std::vector<Date> dates;
        std::vector<Genre> genres;
        std::vector<Family> families;
        std::vector<Media> medias;
        bool available = false;
        // emulationstation compatibility
        std::string source;
        std::string path;
        std::string romsPath;
    };
}

#endif //SS_GAME_H
