//
// Created by cpasjuste on 05/04/19.
//

#ifndef SCREENSCRAP_SS_JEU_H
#define SCREENSCRAP_SS_JEU_H

#include <string>
#include <vector>

namespace ss_api {

    class Game {

    public:

        struct Name {
            std::string country;
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
            struct Name {
                std::string language;
                std::string text;
            };
            std::string id;
            std::string main;
            std::string parentid;
            std::vector<Genre::Name> names;
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
            enum Type {
                SSTitle, SS, Screenshot, Fanart, Video, Marquee,
                ScreenMarquee, ScreenMarqueeSmall, ThemeHs, Manuel,
                Flyer, SteamGrid, Wheel, WheelHD, WheelCarbon, WheelSteel,
                Box2D, Box2DSide, Box2DBack, BoxTexture, Box3D, BoxScan,
                SupportTexture, Bezel43, Bezel169, Bezel1610, Mixrbv1, Mixrbv2,
                Pictoliste, Pictocouleur, Pictomonochrome
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

        std::string id;
        std::string romid;
        std::string notgame;
        std::vector<Name> names;
        std::vector<std::string> countries;
        std::string cloneof;
        std::string systemeid;
        std::string systemename;
        Editor editor;
        Developer developer;
        std::string players;
        std::string rating;
        std::string topstaff;
        std::string rotation;
        std::string resolution;
        std::string inputs;
        std::string colors;
        std::vector<Synopsis> synopsis;
        std::vector<Classification> classifications;
        std::vector<Date> dates;
        std::vector<Genre> genres;
        std::vector<Family> families;
        std::vector<Media> medias;
    };
}

#endif //SCREENSCRAP_SS_JEU_H
