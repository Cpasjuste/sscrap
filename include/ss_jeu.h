//
// Created by cpasjuste on 05/04/19.
//

#ifndef SCREENSCRAP_SS_JEU_H
#define SCREENSCRAP_SS_JEU_H

#include <string>
#include <vector>

namespace ss_api {

    class Jeu {

    public:

        struct Nom {
            std::string region;
            std::string text;
        };

        struct Editeur {
            std::string id;
            std::string text;

        };

        struct Developpeur {
            std::string id;
            std::string text;
        };

        struct Synopsis {
            std::string langue;
            std::string text;
        };

        struct Classification {
            std::string type;
            std::string text;
        };

        struct Date {
            std::string region;
            std::string text;
        };

        struct Genre {
            struct Nom {
                std::string langue;
                std::string text;
            };
            std::string id;
            std::string principale;
            std::string parentid;
            std::vector<Genre::Nom> noms;
        };

        struct Famille {
            struct Nom {
                std::string langue;
                std::string text;
            };
            std::string id;
            std::string principale;
            std::string parentid;
            std::vector<Famille::Nom> noms;
        };

        struct Media {
            std::string type;
            std::string parent;
            std::string url;
            std::string region;
            std::string crc;
            std::string md5;
            std::string sha1;
            std::string format;
        };

        std::string id;
        std::string romid;
        std::string notgame;
        std::vector<Nom> noms;
        std::vector<std::string> regions;
        std::string cloneof;
        std::string systemeid;
        std::string systemenom;
        Editeur editeur;
        Developpeur developpeur;
        std::string joueurs;
        std::string note;
        std::string topstaff;
        std::string rotation;
        std::string resolution;
        std::string controles;
        std::string couleurs;
        std::vector<Synopsis> synopsis;
        std::vector<Classification> classifications;
        std::vector<Date> dates;
        std::vector<Genre> genres;
        std::vector<Famille> familles;
        std::vector<Media> medias;
    };
}

#endif //SCREENSCRAP_SS_JEU_H
