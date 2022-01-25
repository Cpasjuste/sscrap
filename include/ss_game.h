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

        enum class Country : char {
            ALL, DE, ASI, AU, BR, BG, CA, CL,
            CN, AME, KR, CUS, DK, SP, EU,
            FI, FR, GR, HU, IL, IT, JP,
            KW, WOR, MOR, NO, NZ, OCE,
            NL, PE, PL, PT, CZ, UK, RU,
            SS, SK, SE, TW, TR, US,
            UNK
        };

        enum class Language : char {
            EN, FR, ES, PT, ALL, UNK
        };

        struct Name {
            Name() = default;

            Name(const Country &country, const std::string &text) {
                this->country = country;
                this->text = text;
            }

            std::string text;
            Country country = Country::UNK;
        };

        struct System {
            System() = default;

            System(int id, int parentId, const std::string &text) {
                this->id = id;
                this->parentId = parentId;
                this->text = text;
            }

            int id = 0;
            int parentId = 0;
            std::string text;
        };

        struct Editor {
            Editor() = default;

            Editor(int id, const std::string &text) {
                this->id = id;
                this->text = text;
            }

            int id = 0;
            std::string text;
        };

        struct Developer {
            Developer() = default;

            Developer(int id, const std::string &text) {
                this->id = id;
                this->text = text;
            }

            int id = 0;
            std::string text;
        };

        struct Synopsis {
            Synopsis() = default;

            Synopsis(const Language &language, const std::string &text) {
                this->language = language;
                this->text = text;
            }

            std::string text;
            Language language = Language::UNK;
        };

        struct Classification {
            Classification() = default;

            Classification(const std::string &type, const std::string &text) {
                this->type = type;
                this->text = text;
            }

            std::string type;
            std::string text;
        };

        struct Date {
            Date() = default;

            Date(const Country &country, const std::string &text) {
                this->country = country;
                this->text = text;
            }

            std::string text;
            Country country = Country::UNK;
        };

        struct Genre {
            Genre() = default;

            Genre(int id, int mainId, int parentId,
                  const std::string &text, const Language &language) {
                this->id = id;
                this->mainId = mainId;
                this->parentId = parentId;
                this->text = text;
                this->language = language;
            }

            int id = 0;
            int mainId = 0;
            int parentId = 0;
            std::string text;
            Language language = Language::UNK;
        };

        struct Family {
            struct Name {
                std::string text;
                Language language = Language::UNK;
            };
            int id;
            int mainId;
            int parentId;
            std::vector<Family::Name> names;
        };

        struct Media {
            enum class Parent : char {
                All, Game, Country, Editor, Developer,
                Players, Rating, Genre
            };
            std::string parent;
            std::string type;
            std::string url;
            std::string crc;
            std::string md5;
            std::string sha1;
            std::string format;
            std::string support;
            Country country;

            int download(const std::string &dstPath, int retryDelay = 10);
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

        int id = 0;
        int romId = 0;
        int rating = 0;
        int rotation = 0;
        bool notGame = false;
        bool topStaff = false;
        bool available = false;
        std::string cloneOf;
        std::string players;
        std::string resolution;
        //std::string inputs;
        //std::string colors;
        System system;
        Editor editor;
        Developer developer;
        std::vector<Name> names;
        std::vector<Country> countries;
        std::vector<Synopsis> synopses;
        std::vector<Classification> classifications;
        std::vector<Date> dates;
        std::vector<Genre> genres;
        std::vector<Family> families;
        std::vector<Media> medias;
        // emulationstation compatibility
        std::string source;
        std::string path;
        std::string romsPath;
    };
}

#endif //SS_GAME_H
