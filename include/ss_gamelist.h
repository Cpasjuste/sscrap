//
// Created by cpasjuste on 03/12/2019.
//

#ifndef SSCRAP_SS_GAMELIST_H
#define SSCRAP_SS_GAMELIST_H

#include <string>

namespace ss_api {

    class GameList {
    public:

        enum Format {
            ScreenScrapper = 0,
            EmulationStation = 1,
            FbNeo = 2,
            Unknown = 3
        };

        GameList() = default;

        explicit GameList(const std::string &xmlPath, const std::string &romPath = "",
                          bool sort = true, bool addUnknownFiles = false);

        bool append(const std::string &xmlPath, const std::string &romPath = "",
                    bool sort = true, bool addUnknownFiles = false);

        void sortAlpha(bool byZipName = false, bool gamesOnly = true);

        GameList filter(bool available = false, bool clones = false,
                        const std::string &system = "ALL", const std::string &editor = "ALL",
                        const std::string &developer = "ALL", const std::string &player = "ALL",
                        const std::string &rating = "ALL", const std::string &topstaff = "ALL",
                        const std::string &rotation = "ALL", const std::string &resolution = "ALL",
                        const std::string &date = "ALL", const std::string &genre = "ALL");

        GameList filter(bool available = false, bool clones = false,
                        int system = -1, int editor = -1, int developer = -1,
                        int players = -1, int rating = -1, int topstaff = -1,
                        int rotation = -1, const std::string &resolution = "All",
                        const std::string &date = "All", const std::string &genre = "All");

        bool save(const std::string &dstPath,
                  const Game::Language &language = Game::Language::EN,
                  const Format &format = ScreenScrapper, const std::vector<std::string> &mediaList = {});

        std::vector<Game> findGamesByName(const std::string &name);

        std::vector<Game> findGamesByName(const Game &game);

        Game findGameByRomId(long romId);

        Game findGameByPath(const std::string &path);

        Game findGameByPathAndSystem(const std::string &path, int systemId);

        Game::System findSystemByName(const std::string &name);

        std::vector<std::string> getSystemNames();

        Game::Editor findEditorByName(const std::string &name);

        std::vector<std::string> getEditorNames();

        Game::Developer findDeveloperByName(const std::string &name);

        std::vector<std::string> getDeveloperNames();

        std::vector<std::string> getRatingNames();

        std::vector<std::string> getPlayersNames();

        std::vector<std::string> getRotationNames();

        bool exist(long romId);

        bool remove(long romId);

        size_t getAvailableCount();

        std::string xml;
        Format format = EmulationStation;
        std::vector<std::string> romPaths;
        std::vector<Game> games;
        std::vector<Game::System> systems;
        std::vector<Game::Editor> editors;
        std::vector<Game::Developer> developers;
        std::vector<int> players;
        std::vector<int> ratings;
        std::vector<std::string> topStaffs;
        std::vector<int> rotations;
        std::vector<std::string> resolutions;
        std::vector<std::string> dates;
        std::vector<std::string> genres;
    };
}

#endif //SSCRAP_SS_GAMELIST_H
