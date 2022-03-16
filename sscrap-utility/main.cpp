//
// Created by cpasjuste on 29/03/19.
//

#include "ss_api.h"
#include "scrap.h"
#include "args.h"
#include "utility.h"

using namespace ss_api;

static Scrap *scrap;
static int retryDelay = 10;

static void fixFbnGame(Game *ssGame, Game *fbnGame) {
    // game not found in fbneo dat, continue...
    if (fbnGame->path.empty()) {
        return;
    }

    // fix screenscraper "cloneof"
    if (fbnGame->isClone()) {
        ssGame->cloneOf = fbnGame->cloneOf + ".zip";
    }

    // replace screenscraper names by fbneo name
    ssGame->names.clear();
    ssGame->names.emplace_back(Game::Country::SS, fbnGame->names.at(0).text);
}

ss_api::Game *Scrap::getGameByParent(const Io::File &file) {
    const std::string name = file.name;
    auto clone = std::find_if(fbnGameList.games.begin(), fbnGameList.games.end(), [name](const Game &g) {
        return name == g.path;
    });

    if (clone == fbnGameList.games.end() || !clone->isClone()) {
        SS_PRINT("getGameByParent: clone game not found (%s)\n", file.name.c_str());
        return nullptr;
    }

    const std::string parentPath = clone->cloneOf + ".zip";
    auto parent = std::find_if(gameList.games.begin(), gameList.games.end(), [parentPath](const Game &g) {
        return parentPath == g.path;
    });

    if (parent == gameList.games.end()) {
        SS_PRINT("getGameByParent: parent game not found for %s (%s)\n", file.name.c_str(), clone->cloneOf.c_str());
        return nullptr;
    }

    Game *g = new Game(*parent);
    g->path = clone->path;
    g->cloneOf = parentPath;
    g->names.at(0).text = clone->names.at(0).text;
    g->medias.clear();

    return g;
}

bool Scrap::isFbnClone(const Io::File &file) {
    const std::string name = file.name;
    auto game = std::find_if(fbnGameList.games.begin(), fbnGameList.games.end(), [name](const Game &g) {
        return name == g.path;
    });

    return game != gameList.games.end() && game->isClone();
}

// if a custom sscrap custom id is set (fbneo console games),
// we need to use "FinalBurn Neo" databases "description" as rom name
// and map to correct screenscraper systemid
void Scrap::parseSid(int sid) {
    int screenScraperSystemId = sscrapSystemId = sid;

    if (sid == 75 || (sid >= 750 && sid <= 763)) {
        isFbNeoSid = true;
        if (sid == 75) {
            // mame/fbneo
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Arcade only).dat");
        } else if (sid == 750) {
            // colecovision
            screenScraperSystemId = SYSTEM_ID_COLECO;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ColecoVision only).dat");
        } else if (sid == 751) {
            // game gear
            screenScraperSystemId = SYSTEM_ID_GAMEGEAR;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Game Gear only).dat");
        } else if (sid == 752) {
            // master system
            screenScraperSystemId = SYSTEM_ID_SMS;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Master System only).dat");
        } else if (sid == 753) {
            // megadrive
            screenScraperSystemId = SYSTEM_ID_MEGADRIVE;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Megadrive only).dat");
        } else if (sid == 754) {
            // msx
            screenScraperSystemId = SYSTEM_ID_MSX;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, MSX 1 Games only).dat");
        } else if (sid == 755) {
            // pc engine
            screenScraperSystemId = SYSTEM_ID_PCE;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, PC-Engine only).dat");
        } else if (sid == 756) {
            // sega sg-1000
            screenScraperSystemId = SYSTEM_ID_SG1000;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Sega SG-1000 only).dat");
        } else if (sid == 757) {
            // super grafx
            screenScraperSystemId = SYSTEM_ID_SGX;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, SuprGrafx only).dat");
        } else if (sid == 758) {
            // turbo grafx
            screenScraperSystemId = SYSTEM_ID_PCE; // screenscraper doesn't have a turbo grafx section
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, TurboGrafx16 only).dat");
        } else if (sid == 759) {
            // zx spectrum
            screenScraperSystemId = SYSTEM_ID_ZX3;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ZX Spectrum Games only).dat");
        } else if (sid == 760) {
            // nes
            screenScraperSystemId = SYSTEM_ID_NES;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, NES Games only).dat");
        } else if (sid == 761) {
            // nes
            screenScraperSystemId = SYSTEM_ID_NES_FDS;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, FDS Games only).dat");
        } else if (sid == 762) {
            // nes
            screenScraperSystemId = SYSTEM_ID_CHANNELF;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Fairchild Channel F Games only).dat");
        } else if (sid == 763) {
            // nes
            screenScraperSystemId = SYSTEM_ID_NGP;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, NeoGeo Pocket Games only).dat");
        }
    }

    system = systemList.findById(std::to_string(screenScraperSystemId));
}

ss_api::Game Scrap::scrapGame(int tid, int tryCount, int sid, int remainingFiles, const std::string &fileName,
                              const std::string &filePath, const std::string &searchName) {
    Game game = {};
    Game fbnGame = {};
    GameInfo gameInfo = {};
    std::string searchType = "none";
    std::string fileCrc, romCrc, romType;
    bool isZip = Io::endsWith(fileName, ".zip", false);
    bool isIso = Io::endsWith(fileName, ".iso", false);

    if (isZip) {
        romType = "rom";
    } else if (isIso) {
        romType = "iso";
    }

    // first, search by zip crc
    fileCrc = Utility::getFileCrc(filePath);
    if (!fileCrc.empty()) {
        gameInfo = GameInfo(fileCrc, "", "", std::to_string(sid), romType,
                            fileName, "", "", usr, pwd, retryDelay);
        if (gameInfo.http_error == 0) {
            searchType = "file_crc";
        } else if (gameInfo.http_error == 430 || gameInfo.http_error == 431 || gameInfo.http_error == 500) {
            Api::printc(COLOR_R, "NOK: thread[%i] => Quota reached for today... "
                                 "See https://www.screenscraper.fr if you want to support "
                                 "screenscraper and maximize your quota!\n", tid);
            return game;
        }
    }
    SS_PRINT("game_info (file_crc): %s (%s), res = %i\n", searchName.c_str(), fileCrc.c_str(), gameInfo.http_error);

    // next, try by rom crc if not mame/fbneo (multiple roms in zip...)
    if (gameInfo.http_error != 0 && isZip && sid != 75) {
        romCrc = Utility::getRomCrc(filePath);
        if (!romCrc.empty()) {
            gameInfo = GameInfo(romCrc, "", "", std::to_string(sid), romType,
                                fileName, "", "", usr, pwd, retryDelay);
            if (gameInfo.http_error == 0) {
                searchType = "rom_crc";
            } else if (gameInfo.http_error == 430 || gameInfo.http_error == 431 || gameInfo.http_error == 500) {
                Api::printc(COLOR_O, "NOK: thread[%i] => Quota reached for today... "
                                     "See https://www.screenscraper.fr if you want to support "
                                     "screenscraper and maximize your quota!\n", tid);
                return game;
            }
        }
        SS_PRINT("game_info (rom_crc): %s (%s), res = %i\n", searchName.c_str(), romCrc.c_str(), gameInfo.http_error);
    }

    // fbneo consoles zip names doesn't match standard consoles zip names
    // this will also help fbneo arcade games if not found by zip name
    if (isFbNeoSid) {
        fbnGame = fbnGameList.findGameByPath(fileName);
    }

    // finally, try a game search (jeuRecherche)
    if (gameInfo.http_error != 0) {
        // the rom is not know by screenscraper, try to find the game with a game search (jeuRecherche)
        std::string name = isFbNeoSid ? fbnGame.getName().text : Utility::removeExt(searchName);
        // remove "(xxx)"
        size_t pos = name.find_first_of('(');
        if (pos != std::string::npos && pos > 2) {
            name = name.substr(0, pos - 1);
        }
        GameSearch search = GameSearch(name, std::to_string(sid), usr, pwd, retryDelay);
        SS_PRINT("game_search: %s, res = %i\n", name.c_str(), gameInfo.http_error);
        if (!search.games.empty()) {
            int id = sid;
            auto g = std::find_if(search.games.begin(), search.games.end(), [id, name](const Game &g) {
                return (g.system.id == id || g.system.parentId == id)
                       && g.getName().text.find(name) != std::string::npos;
            });
            if (g != search.games.end()) {
                searchType = "game_search";
                gameInfo.http_error = 0;
                gameInfo.game = *g;
                gameInfo.game.path = fileName;
            }
        }
    }

    if (isFbNeoSid) {
        fixFbnGame(&gameInfo.game, &fbnGame);
    }

    if (gameInfo.http_error == 0 && gameInfo.game.id > 0) {
        // TODO: clones are not scrapped anymore, remove parent/clone stuff
        // process medias download
        bool processMedia = args.exist("-dlm");
        bool useParentMedia;
        if (processMedia) {
            // if rom media was already scrapped for a same "screenscraper game", skip it
            // this is useful for non arcade roms for which clone notion doesn't exist
            if (!args.exist("-dlmc")) {
                const std::string name = gameInfo.game.getName().text;
                auto it = std::find_if(namesList.begin(), namesList.end(),
                                       [name](const std::string &n) {
                                           return n == name;
                                       });
                if (it != namesList.end()) {
                    processMedia = false;
                }
            }

            // now check for clones (replace medias path with parent medias path)
            useParentMedia = !args.exist("-dlmc") && gameInfo.game.isClone();

            // push game name to list
            pthread_mutex_lock(&mutex);
            namesList.emplace_back(gameInfo.game.getName().text);
            pthread_mutex_unlock(&mutex);

            // process...
            if (processMedia) {
                std::string mediaPath = romPath + "/media/";
                if (!Io::exist(mediaPath) && !useParentMedia) {
                    Io::makedir(mediaPath);
                }

                for (const auto &mediaType: mediasGameList.medias) {
                    // if media type is not in args, skip it
                    if (!args.exist(mediaType.nameShort)) {
                        continue;
                    }

                    if (useParentMedia) {
                        for (int i = 0; i < gameInfo.game.medias.size(); i++) {
                            if (gameInfo.game.medias.at(i).type == mediaType.nameShort) {
                                std::string ext = gameInfo.game.medias.at(i).format;
                                gameInfo.game.medias.at(i).url =
                                        "media/" + mediaType.nameShort + "/"
                                        + Utility::removeExt(gameInfo.game.cloneOf) + "." + ext;
                            }
                        }
                        continue;
                    }

                    Game::Media media = gameInfo.game.getMedia(mediaType.nameShort, Game::Country::SS);
                    if (media.url.empty()) {
                        continue;
                    }

                    std::string mediaName, mediaNameRoq;
                    if (fileName.find_last_of('.') != std::string::npos) {
                        std::string noExt = fileName.substr(0, fileName.find_last_of('.') + 1);
                        mediaName = noExt + media.format;
                        mediaNameRoq = noExt + "roq";
                    } else {
                        mediaName = fileName + "." + media.format;
                        mediaNameRoq = fileName + ".roq";
                    }

                    std::string path = mediaPath + media.type + "/";
                    if (!Io::exist(path)) {
                        Io::makedir(path);
                    }

                    // skip if media already exists
                    if (Io::exist(path + mediaName)) {
                        SS_PRINT("MDL: SKIP: %s\n", (path + mediaName).c_str());
                        continue;
                    }

                    // dc: skip if ".roq" converted video exists
                    if (sid == SYSTEM_ID_DREAMCAST || sid == SYSTEM_ID_ATOMISWAVE) {
                        if (media.format == "mp4" && Io::exist(path + mediaNameRoq)) {
                            SS_PRINT("MDL: SKIP: %s\n", (path + mediaNameRoq).c_str());
                            continue;
                        }
                    }

                    media.download(path + mediaName);
                }
            }
        }

        // fix missing tg16 system in screenscraper (for fbneo)
        if (sscrapSystemId == SYSTEM_ID_TG16) {
            gameInfo.game.system.id = SYSTEM_ID_TG16;
            gameInfo.game.system.parentId = SYSTEM_ID_PCE;
            gameInfo.game.system.text = "PC Engine TurboGrafx";
        }

        const char *color = searchType == "game_search" ? COLOR_Y : COLOR_G;
        Api::printc(color, "[%i/%i] OK: %s => %s (%s) (%s)\n",
                    filesCount - remainingFiles, filesCount,
                    fileName.c_str(), gameInfo.game.getName().text.c_str(),
                    gameInfo.game.system.text.c_str(), searchType.c_str());

        return gameInfo.game;
    } else {
        // game not found, but add it to the list with default values
        if (isFbNeoSid) {
            game = fbnGame;
            game.id = game.romId = std::stol(fileCrc, nullptr, 16);
            game.system.id = sid;
            game.system.text = system.names.eu;
            // fix missing tg16 system in screenscraper (for fbneo)
            if (sscrapSystemId == SYSTEM_ID_TG16) {
                game.system.id = SYSTEM_ID_TG16;
                game.system.parentId = SYSTEM_ID_PCE;
                game.system.text = "PC Engine TurboGrafx";
            }
        } else {
            game.names.emplace_back(Game::Country::WOR, searchName);
            game.id = game.romId = std::stol(fileCrc, nullptr, 16);
            game.system.id = sid;
            game.system.text = system.names.eu;
            game.path = searchName;
        }

        if (isFbNeoSid) {
            Api::printc(COLOR_R, "[%i/%i] NOK: %s (%s) (%i)\n",
                        filesCount - remainingFiles, filesCount,
                        searchName.c_str(), fbnGame.getName().text.c_str(), gameInfo.http_error);
            pthread_mutex_lock(&mutex);
            missList.emplace_back(game.getName().text, game.path, fileCrc, romCrc);
            pthread_mutex_unlock(&mutex);
        } else {
            if (sid == SYSTEM_ID_DREAMCAST && tryCount < 3) {
                // wait for 2nd and 3nd try
            } else {
                Api::printc(COLOR_R, "[%i/%i] NOK: %s (%i)\n",
                            filesCount - remainingFiles, filesCount,
                            searchName.c_str(), gameInfo.http_error);
                pthread_mutex_lock(&mutex);
                missList.emplace_back(game.getName().text, game.path, fileCrc, romCrc);
                pthread_mutex_unlock(&mutex);
            }
        }

        return game;
    }
}

static void *scrap_thread(void *ptr) {
    int tid = *((int *) ptr);

    while (true) {

        pthread_mutex_lock(&scrap->mutex);
        if (scrap->filesList.empty()) {
            pthread_mutex_unlock(&scrap->mutex);
            break;
        }

        Io::File file = scrap->filesList.at(0);
        scrap->filesList.erase(scrap->filesList.begin());
        int remainingFiles = (int) scrap->filesList.size();
        pthread_mutex_unlock(&scrap->mutex);

        Game game = {};
        int try_count = 1;
        // dc scrapping is "special"
        if (scrap->system.getId() == SYSTEM_ID_DREAMCAST) {
            game = scrap->scrapGame(tid, try_count, scrap->system.getId(),
                                    remainingFiles, file.name, file.path, file.dc_header_title);
            try_count++;
        }

        if (game.id == 0) {
            game = scrap->scrapGame(tid, try_count, scrap->system.getId(),
                                    remainingFiles, file.name, file.path, file.name);
            try_count++;
        }

        // // dc scrapping is "special", try atomiswave system
        if (game.id == 0 && scrap->system.getId() == SYSTEM_ID_DREAMCAST) {
            game = scrap->scrapGame(tid, try_count, SYSTEM_ID_ATOMISWAVE,
                                    remainingFiles, file.name, file.path, file.dc_header_title);
        }

        // add the game to game list
        if (!game.names.empty()) {
            pthread_mutex_lock(&scrap->mutex);
            scrap->gameList.games.emplace_back(game);
            pthread_mutex_unlock(&scrap->mutex);
        }
    }

    return nullptr;
}

Scrap::Scrap(const ArgumentParser &parser) {
    args = parser;

    // setup screenscraper api
#ifdef _MSC_VER
    Api::ss_devid = "";
    Api::ss_devpassword = "";
#else
    Api::ss_devid = SS_DEV_ID;
    Api::ss_devpassword = SS_DEV_PWD;
#endif
    Api::ss_softname = "sscrap";
    ss_debug = args.exist("-d");

    usr = args.get("-u");
    pwd = args.get("-p");

    // banner
    Api::printc(COLOR_G, "  _________ ____________________________    _____ __________ \n");
    Api::printc(COLOR_G, " /   _____//   _____/\\_   ___ \\______   \\  /  _  \\\\______   \\\n");
    Api::printc(COLOR_G, " \\_____  \\ \\_____  \\ /    \\  \\/|       _/ /  /_\\  \\|     ___/\n");
    Api::printc(COLOR_G, " /        \\/        \\\\     \\___|    |   \\/    |    \\    |    \n");
    Api::printc(COLOR_G, "/_______  /_______  / \\______  /____|_  /\\____|__ _/____|    \n");
    Api::printc(COLOR_G, "        \\/        \\/         \\/       \\/ 2020 @ cpasjuste\n\n");

    if (!args.tokens.empty() && !args.exist("-zi")) {
        Api::printc(COLOR_G, "Getting user information... ");
        user = User(usr, pwd);
        Api::printc(COLOR_G, "found user %s, threads: %s, requests: %s/%s, download speed: %s Ko/s\n",
                    user.id.c_str(), user.maxthreads.c_str(),
                    user.requeststoday.c_str(), user.maxrequestsperday.c_str(),
                    user.maxdownloadspeed.c_str());

        Api::printc(COLOR_G, "Updating systems... ");
        systemList = SystemList(usr, pwd, retryDelay);
        Api::printc(COLOR_G, "found %zu systems\n", systemList.systems.size());

        Api::printc(COLOR_G, "Updating medias types... ");
        mediasGameList = MediasGameList(usr, pwd, retryDelay);
        Api::printc(COLOR_G, "found %zu medias type\n", mediasGameList.medias.size());
    }
}

void Scrap::run() {
    if (user.http_error == 430 || user.http_error == 431 || user.http_error == 500) {
        Api::printc(COLOR_R, "NOK: Quota reached for today... "
                             "See https://www.screenscraper.fr if you want to support "
                             "screenscraper and maximize your quota!\n\n");
        return;
    }

    if (systemList.http_error == 430 || systemList.http_error == 431 || systemList.http_error == 500) {
        Api::printc(COLOR_R, "NOK: Quota reached for today... "
                             "See https://www.screenscraper.fr if you want to support "
                             "screenscraper and maximize your quota!\n\n");
        return;
    }

    if (mediasGameList.http_error == 430 || mediasGameList.http_error == 431 || mediasGameList.http_error == 500) {
        Api::printc(COLOR_R, "NOK: Quota reached for today... "
                             "See https://www.screenscraper.fr/ if you want to support "
                             "screenscraper and maximize your quota!\n\n");
        return;
    }

    parseSid(Utility::parseInt(args.get("-sid")));

    if (args.exist("-r")) {
        romPath = args.get("-r");
        Api::printc(COLOR_G, "Building roms list... ");
        std::vector<std::string> filters = {".zip"};
        if (args.exist("-filter")) {
            filters = {args.get("-filter")};
        }
        if (system.getId() == SYSTEM_ID_DREAMCAST) {
            filesList = Io::getDirList(romPath, true, {".gdi"});
        } else {
            filesList = Io::getDirList(romPath, false, filters);
        }
        filesCount = (int) filesList.size();

        Api::printc(COLOR_G, "found %zu roms\n", filesCount);
        if (filesList.empty()) {
            Api::printc(COLOR_R, "ERROR: no files found in rom path\n");
            return;
        }

        //SystemList::System system = systemList.findById(std::to_string(systemId));
        Api::printc(COLOR_G, "Scrapping system '%s', let's go!\n\n", system.names.eu.c_str());

        // if fbneo/mame system filter clones to process them later with parent game
        Api::printc(COLOR_G, "\nSkipping clones, will use parent games...\n");
        if (isFbNeoSid) {
            for (int i = filesCount - 1; i > -1; i--) {
                if (isFbnClone(filesList.at(i))) {
                    cloneList.push_back(filesList.at(i));
                    filesList.erase(filesList.begin() + i);
                }
            }
        }

        pthread_mutex_init(&mutex, nullptr);
        int maxThreads = user.getMaxThreads();

        for (int i = 0; i < maxThreads; i++) {
            pthread_create(&threads[i], nullptr, scrap_thread, &i);
        }

        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threads[i], nullptr);
        }

        pthread_mutex_destroy(&mutex);

        // if fbneo/mame system process clones now based on parent game
        if (isFbNeoSid) {
            Api::printc(COLOR_G, "\nPlease wait, processing clones...\n");
            for (auto &i: cloneList) {
                Game *game = getGameByParent(i);
                // should always be found as we found it before...
                if (game) {
                    gameList.games.emplace_back(*game);
                }
            }
        }

        if (!gameList.games.empty()) {
            Game::Language lang = Game::Language::EN;
            if (args.exist("fr")) {
                lang = Game::Language::FR;
            } else if (args.exist("es")) {
                lang = Game::Language::ES;
            } else if (args.exist("pt")) {
                lang = Game::Language::PT;
            }

            // save emulationstation gamelist.xml
            std::vector<std::string> mediaList;
            mediaList.emplace_back(args.get("-i"));
            mediaList.emplace_back(args.get("-t"));
            mediaList.emplace_back(args.get("-v"));
            gameList.save(romPath + "/gamelist.xml", lang, GameList::Format::EmulationStation, mediaList);

            // save pemu gamelist.xml
            mediaList.clear();
            for (const auto &mediaType: mediasGameList.medias) {
                if (args.exist(mediaType.nameShort)) {
                    mediaList.emplace_back(mediaType.nameShort);
                }
            }
            gameList.save(romPath + "/gamelist_pemu.xml", lang, GameList::Format::ScreenScrapper, mediaList);
        }
        // print results
        FILE *f = fopen("sscrap.log", "w+");
        Api::printc(COLOR_G, "\nAll Done... ");
        Api::printc(COLOR_G, "found %zu/%i games\n", gameList.games.size() - missList.size(), filesCount);
        if (f) {
            fprintf(f, "Found %zu/%i games\n", gameList.games.size() - missList.size(), filesCount);
        }
        if (!missList.empty()) {
            Api::printc(COLOR_O, "\n%zu game(s) not found:\n", missList.size());
            if (f) {
                fprintf(f, "\n%zu game(s) not found:\n", missList.size());
            }
            for (const auto &miss: missList) {
                std::string missInfo = Utility::getZipInfoStr(romPath + "/", miss.path);
                Api::printc(COLOR_R, "%s (%s)\n", missInfo.c_str(), miss.name.c_str());
                if (f) {
                    fprintf(f, "%s\n", missInfo.c_str());
                }
            }
        }
        printf("\n");
        if (f) {
            fclose(f);
        }
    } else if (args.exist("-ml")) {
        Api::printc(COLOR_G, "\nAvailable screenscraper medias types:\n\n");
        for (const auto &media: mediasGameList.medias) {
            Api::printc(COLOR_G, "\t%s (type: %s, category: %s)\n",
                        media.nameShort.c_str(), media.type.c_str(), media.category.c_str());
        }
        printf("\n");
    } else if (args.exist("-sl")) {
        Api::printc(COLOR_G, "\nAvailable screenscraper systems:\n\n");
        for (const auto &s: systemList.systems) {
            Api::printc(COLOR_G, "\t%s (id: %s, company: %s, type: %s)\n",
                        s.names.eu.c_str(), s.id.c_str(), s.company.c_str(), s.type.c_str());
        }
        printf("\n");
    } else if (args.exist("-zi")) {
        const std::string fullPath = args.get("-zi");
        size_t sep = fullPath.rfind('/');
        if (sep != std::string::npos) {
            std::string zipInfo = Utility::getZipInfoStr(fullPath.substr(0, sep),
                                                         fullPath.substr(sep + 1, fullPath.size()));
            Api::printc(COLOR_R, "%s\n", zipInfo.c_str());
        }
    } else {
        printf("\n");
        printf("usage: sscrap -u <screenscraper_user> -p <screenscraper_password> [options] [options_es]\n");
        printf("\n\toptions:\n");
        printf("\t\t-d                             enable debug output\n");
        printf("\t\t-sl                            list available screenscraper systems and exit\n");
        printf("\t\t-ml                            list available screenscraper medias types and exit\n");
        printf("\t\t-zi <rom_path>                 show zip information (size, crc, md5, sha1) and exit\n");
        printf("\t\t-sid <system_id>               screenscraper system id to scrap\n");
        printf("\t\t-r <roms_path>                 path to roms files to scrap\n");
        printf("\t\t-filter <ext>                  only scrap files with this extension\n");
        printf("\t\t-dlm <mediaType1 mediaType2>   download given medias types\n");
        printf("\t\t-dlmc                          download medias for clones\n");
        printf("\t\t-lang <language>               use given output language for gamelist.xml\n");
        printf("\n\toptions_es:\n");
        printf("\t\t-i <mediaType>                 use given media type for image\n");
        printf("\t\t-t <mediaType>                 use given media type for thumbnail\n");
        printf("\t\t-v <mediaType>                 use given media type for video\n");
        printf("\n\tsscrap supported output languages:\n");
        printf("\t\ten: english, default\n");
        printf("\t\tfr: french\n");
        printf("\t\tes: spanish\n");
        printf("\t\tpt: portuguese\n");
        printf("\n\tsscrap customs systemid (fbneo):\n");
        printf("\t\t750: ColecoVision\n");
        printf("\t\t751: Game Gear\n");
        printf("\t\t752: Master System\n");
        printf("\t\t753: Megadrive\n");
        printf("\t\t754: MSX 1\n");
        printf("\t\t755: NEC PC-Engine\n");
        printf("\t\t756: Sega SG-1000\n");
        printf("\t\t757: NEC Super Grafx\n");
        printf("\t\t758: NEC Turbo Grafx\n");
        printf("\t\t759: ZX Spectrum\n");
        printf("\t\t760: Nintendo NES\n");
        printf("\t\t761: Nintendo NES FDS\n");
        printf("\t\t762: Fairchild Channel F\n");
        printf("\t\t763: NeoGeo Pocket\n");
        printf("\n");
        printf("examples:\n\n");
        printf("\tscrap mame/fbneo system, download \'mixrbv2\' for \'image\', \'box-3D\' for \'thumbnail\' and \'video\' for \'video\':\n");
        printf("\t\tsscrap -u user -p password -r /roms -sid 75 -dlm mixrbv2 box-3D video -i mixrbv2 -t box-3D -v video\n\n");
        printf("\n");
    }
}

int main(int argc, char *argv[]) {

    ArgumentParser args(argc, argv);
    scrap = new Scrap(args);
    scrap->run();

    return 0;
}
