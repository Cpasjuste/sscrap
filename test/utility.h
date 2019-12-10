//
// Created by cpasjuste on 10/12/2019.
//

#ifndef SSCRAP_UTILITY_H
#define SSCRAP_UTILITY_H

#include <string>
#include <vector>
#include <ss_game.h>

class Utility {
public:

    static std::string getExt(const std::string &file);

    static std::string getZipCrc(const std::string &zipPath);

    static std::string getRomCrc(const std::string &zipPath, std::vector<std::string> whiteList = {});

    static void printGame(const ss_api::Game &game);
};

#endif //SSCRAP_UTILITY_H
