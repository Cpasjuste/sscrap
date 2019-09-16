//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_GAME_H
#define SS_GAME_H

#include <string>
#include <vector>

namespace ss_api {

    class Game {

    public:

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
            std::string country;
            std::string text;
        };

        struct System {
            std::string id;
            std::string text;
        };

        struct Editor {
            std::string id;
            std::string text;
        };

        struct Developer {
            std::string id;
            std::string text;
        };

        struct Synopsis {
            std::string language;
            std::string text;
        };

        struct Classification {
            std::string type;
            std::string text;
        };

        struct Date {
            std::string country;
            std::string text;
        };

        struct Genre {
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
            enum class Type {
                SSTitle, SS, Screenshot, Fanart, Video, Marquee,
                ScreenMarquee, ScreenMarqueeSmall, ThemeHs, Manuel,
                Flyer, SteamGrid, Wheel, WheelHD, WheelCarbon, WheelSteel,
                Box2D, Box2DSide, Box2DBack, BoxTexture, Box3D, BoxScan,
                SupportTexture, Bezel43, Bezel169, Bezel1610, Mixrbv1, Mixrbv2,
                Pictoliste, Pictocouleur, Pictomonochrome,
                Unknow
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
        };

        Game::Name getName(const Game::Country &country) const;

        Game::Synopsis getSynopsis(const Game::Language &language) const;

        // TODO: to lazy for now
        //Game::Classification getClassification(const Game::Country &country) const;

        Game::Date getDate(const Game::Country &country) const;

        Game::Genre getGenre(const Game::Language &language) const;

        // TODO: to lazy for now
        //Game::Family::Name getFamily(const Game::Language &language) const;

        Game::Media getMedia(const Game::Media::Type &type, const Game::Country &country) const;

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
        // emulationstation compatibility
        std::string source;
        std::string path;
    };
}

#endif //SS_GAME_H
