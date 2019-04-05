//
// Created by cpasjuste on 05/04/19.
//

#ifndef SCREENSCRAP_S_GAME_H
#define SCREENSCRAP_S_GAME_H

#include <string>
#include <vector>

#include "s_country.h"

class Game {

public:

    class Name {
    public:
        std::string region;
        std::string text;
    };

    class Country {
    public:
        std::string shortname;
    };

    class Editor {
    public:
        int id;
        std::string text;

    };

    class Developer {
    public:
        int id;
        std::string text;
    };

    class System {
    public:
        int id;
        std::string name;
    };

    class Synopsis {
    public:
        std::string language;
        std::string text;
    };

    class Classification {
        std::string type;
        std::string text;
    };

    int id;
    int rom_id;
    bool not_game;
    std::vector<Name> names;
    std::vector<Country> countries;
    int clone_of;
    System system;
    Editor editor;
    Developer developer;
    std::string players;
    int rating;
    int top_staff;
    int rotation;
    std::string resolution;
    std::string controls;
    std::string colors;
    std::vector<Synopsis> synopses;
    
}

#endif //SCREENSCRAP_S_GAME_H
