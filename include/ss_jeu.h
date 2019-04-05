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
            struct Name {
                std::string langue;
                std::string text;
            };
            std::string id;
            std::vector<Genre::Name> names;
        };

        struct Famille {
            struct Name {
                std::string langue;
                std::string text;
            };
            std::string id;
            std::vector<Genre::Name> names;
        };

        std::string id;
        std::string romid;
        std::string notgame;
        std::vector<Nom> noms;
        std::vector<std::string> regionshortnames;
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
        std::vector<Date> dates;
        std::vector<Famille> familles;

    };
}

#endif //SCREENSCRAP_SS_JEU_H
