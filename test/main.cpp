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

static void fixFbnClone(Game *game, const GameList &list) {

    // mame/fbneo: search fbn list for zip name (path), as game name may differ
    const std::string zipName = game->path;
    auto fbaGame = std::find_if(list.games.begin(), list.games.end(), [zipName](const Game &g) {
        return zipName == g.path && g.isClone();
    });
    // screenscraper game not found in fbneo dat, or is not a clone continue...
    if (fbaGame == list.games.end()) {
        return;
    }
    // fix screenscrapecloneof !
    game->cloneOf = (*fbaGame).cloneOf + ".zip";
}

// if a custom sscrap custom id is set (fbneo console games),
// we need to use "FinalBurn Neo" databases "description" as rom name
// and map to correct screenscraper systemid
void Scrap::parseSid(int sid) {

    systemId = systemIdFbNeo = sid;
    if (sid == 75 || (sid >= 750 && sid <= 761)) {
        isFbNeoSid = true;
        if (sid == 75) {
            // mame/fbneo
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Arcade only).dat");
        } else if (sid == 750) {
            // colecovision
            systemId = SYSTEM_ID_COLECO;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ColecoVision only).dat");
        } else if (sid == 751) {
            // game gear
            systemId = SYSTEM_ID_GAMEGEAR;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Game Gear only).dat");
        } else if (sid == 752) {
            // master system
            systemId = SYSTEM_ID_SMS;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Master System only).dat");
        } else if (sid == 753) {
            // megadrive
            systemId = SYSTEM_ID_MEGADRIVE;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Megadrive only).dat");
        } else if (sid == 754) {
            // msx
            systemId = SYSTEM_ID_MSX;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, MSX 1 Games only).dat");
        } else if (sid == 755) {
            // pc engine
            systemId = SYSTEM_ID_PCE;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, PC-Engine only).dat");
        } else if (sid == 756) {
            // sega sg-1000
            systemId = SYSTEM_ID_SG1000;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Sega SG-1000 only).dat");
        } else if (sid == 757) {
            // super grafx
            systemId = SYSTEM_ID_SGX;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, SuprGrafx only).dat");
        } else if (sid == 758) {
            // turbo grafx
            systemId = SYSTEM_ID_PCE;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, TurboGrafx16 only).dat");
        } else if (sid == 759) {
            // zx spectrum
            systemId = SYSTEM_ID_ZX3;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ZX Spectrum Games only).dat");
        } else if (sid == 760) {
            // nes
            systemId = SYSTEM_ID_NES;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, NES Games only).dat");
        } else if (sid == 761) {
            // nes
            systemId = SYSTEM_ID_NES_FDS;
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, FDS Games only).dat");
        }
    }
}

static void *scrap_thread(void *ptr) {

    int tid = *((int *) ptr);
    Game fbnGame;

    while (!scrap->filesList.empty()) {

        pthread_mutex_lock(&scrap->mutex);
        std::string file = scrap->filesList.at(0);
        scrap->filesList.erase(scrap->filesList.begin());
        int filesSize = (int) scrap->filesList.size();
        pthread_mutex_unlock(&scrap->mutex);

        GameInfo gameInfo;
        std::string searchType = "None";
        std::string zipCrc, romCrc;

        // first, search by zip crc
        std::string path = scrap->romPath + "/" + file;
        zipCrc = Utility::getZipCrc(path);
        if (!zipCrc.empty()) {
            gameInfo = GameInfo(zipCrc, "", "", std::to_string(scrap->systemId), "", file,
                                "", "", scrap->usr, scrap->pwd, retryDelay);
            if (gameInfo.http_error == 0) {
                searchType = "zip_crc";
            } else if (gameInfo.http_error == 430 || gameInfo.http_error == 431 || gameInfo.http_error == 500) {
                Api::printc(COLOR_R, "NOK: thread[%i] => Quota reached for today... "
                                     "See https://www.screenscraper.fr if you want to support "
                                     "screenscraper and maximize your quota!\n", tid);
                break;
            }
        }
        SS_PRINT("zip crc: %s (%s), res = %i\n", file.c_str(), zipCrc.c_str(), gameInfo.http_error);

        // next, try by rom crc if not mame / fbneo arcade (multiple roms in zip...)
        if (gameInfo.http_error != 0 && scrap->systemId != 75) {
            path = scrap->romPath + "/" + file;
            romCrc = Utility::getRomCrc(path);
            if (!romCrc.empty()) {
                gameInfo = GameInfo(romCrc, "", "", std::to_string(scrap->systemId), "", file,
                                    "", "", scrap->usr, scrap->pwd, retryDelay);
                if (gameInfo.http_error == 0) {
                    searchType = "rom_crc";
                } else if (gameInfo.http_error == 430 || gameInfo.http_error == 431 || gameInfo.http_error == 500) {
                    Api::printc(COLOR_O, "NOK: thread[%i] => Quota reached for today... "
                                         "See https://www.screenscraper.fr if you want to support "
                                         "screenscraper and maximize your quota!\n", tid);
                    break;
                }
            }
            SS_PRINT("rom crc: %s (%s), res = %i\n", file.c_str(), romCrc.c_str(), gameInfo.http_error);
        }

        // fbneo consoles zip names doesn't match standard consoles zip names
        // this will also help fbneo arcade games if not found by zip name
        if (scrap->isFbNeoSid) {
            fbnGame = scrap->fbnGameList.findByPath(file);
        }

        // now try with zip name
        if (gameInfo.http_error != 0) {
            std::string name = (scrap->isFbNeoSid && scrap->systemId != 75) ? fbnGame.getName().text + ".zip" : file;
            gameInfo = GameInfo("", "", "", std::to_string(scrap->systemId), "rom", name,
                                "", "", scrap->usr, scrap->pwd, retryDelay);
            if (gameInfo.http_error == 0) {
                searchType = "zip_name";
            }
            if (scrap->isFbNeoSid && scrap->systemId != 75) {
                gameInfo.game.path = file;
            }
            SS_PRINT("zip name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
        }

        // finally, try a game search (jeuRecherche)
        if (gameInfo.http_error != 0) {
            // the rom is not know by screenscraper, try to find the game with a game search (jeuRecherche)
            std::string name = scrap->isFbNeoSid ? fbnGame.getName().text : file;
            GameSearch search = GameSearch(name, std::to_string(scrap->systemId), scrap->usr, scrap->pwd, retryDelay);
            SS_PRINT("search name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
            if (!search.games.empty()) {
                int id = scrap->systemId;
                auto game = std::find_if(search.games.begin(), search.games.end(), [id](const Game &game) {
                    return game.system.id == id || game.system.parentId == id;
                });
                if (game != search.games.end()) {
                    gameInfo = GameInfo("", "", "", "", "", file,
                                        "", std::to_string((*game).id), scrap->usr, scrap->pwd, retryDelay);
                    if (gameInfo.http_error == 0) {
                        searchType = "search";
                    }
                    SS_PRINT("search name (romid): %s, res = %i\n", name.c_str(), gameInfo.http_error);
                }
            } else {
                // last try, remove "(xxx)"
                size_t pos = name.find_first_of('(');
                if (pos != std::string::npos) {
                    name = name.substr(0, pos - 1);
                    search = GameSearch(name, std::to_string(scrap->systemId), scrap->usr, scrap->pwd, retryDelay);
                    SS_PRINT("search name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
                    if (!search.games.empty()) {
                        int id = scrap->systemId;
                        auto game = std::find_if(search.games.begin(), search.games.end(), [id](const Game &game) {
                            return game.system.id == id || game.system.parentId == id;
                        });
                        if (game != search.games.end()) {
                            gameInfo = GameInfo("", "", "", "", "", file,
                                                "", std::to_string((*game).id), scrap->usr, scrap->pwd, retryDelay);
                            if (gameInfo.http_error == 0) {
                                searchType = "search";
                            }
                            SS_PRINT("search name (romid): %s, res = %i\n", name.c_str(), gameInfo.http_error);
                        }
                    }
                }
            }
        }

        if (scrap->isFbNeoSid) {
            fixFbnClone(&gameInfo.game, scrap->fbnGameList);
        }

        if (gameInfo.http_error == 0) {
            // process medias download
            bool processMedia = scrap->args.exist("-dlm") && (!scrap->mediasClone && !gameInfo.game.isClone());
            if (processMedia) {
                // if rom media was already scrapped for a same "screenscraper game", skip it
                // this is useful for non arcade roms for which clone notion doesn't exist
                if (!scrap->mediasClone) {
                    const std::string name = gameInfo.game.getName().text;
                    auto it = std::find_if(scrap->namesList.begin(), scrap->namesList.end(),
                                           [name](const std::string &n) {
                                               return n == name;
                                           });
                    if (it != scrap->namesList.end()) {
                        processMedia = false;
                    }
                }
                pthread_mutex_lock(&scrap->mutex);
                scrap->namesList.emplace_back(gameInfo.game.getName().text);
                pthread_mutex_unlock(&scrap->mutex);

                if (processMedia) {
                    for (const auto &mediaType : scrap->mediasGameList.medias) {
                        // if media type is not in args, skip it
                        if (!scrap->args.exist(mediaType.nameShort)) {
                            continue;
                        }
                        Game::Media media = gameInfo.game.getMedia(mediaType.nameShort, Game::Country::SS);
                        if (media.url.empty()) {
                            continue;
                        }
                        std::string name = file.substr(0, file.find_last_of('.') + 1) + media.format;
                        path = scrap->romPath + "/media/" + media.type + "/";
                        if (!Io::exist(path)) {
                            Io::makedir(path);
                        }
                        path += name;
                        if (Io::exist(path)) {
                            continue;
                        }
                        media.download(path);
                        //std::string src = "\"/home/cpasjuste/emulation/media/" + media.type + "/" + name + "\"";
                        //std::string cmd = "cp " + src + " \"" + path + "\"";
                        //system(cmd.c_str());
                    }
                }
            }

            pthread_mutex_lock(&scrap->mutex);
            // pFBN custom id
            if (scrap->systemIdFbNeo == SYSTEM_ID_TG16) {
                gameInfo.game.system.id = SYSTEM_ID_TG16;
                gameInfo.game.system.parentId = SYSTEM_ID_PCE;
                gameInfo.game.system.text = "PC Engine TurboGrafx";
            }
            if (searchType == "search") {
                Api::printc(COLOR_Y, "[%i/%i] OK: %s => %s (%s) (%s)\n",
                            scrap->filesCount - filesSize, scrap->filesCount,
                            file.c_str(), gameInfo.game.getName().text.c_str(),
                            gameInfo.game.system.text.c_str(), searchType.c_str());
            } else {
                Api::printc(COLOR_G, "[%i/%i] OK: %s => %s (%s) (%s)\n",
                            scrap->filesCount - filesSize, scrap->filesCount,
                            file.c_str(), gameInfo.game.getName().text.c_str(),
                            gameInfo.game.system.text.c_str(), searchType.c_str());
            }
            scrap->gameList.games.emplace_back(gameInfo.game);
            pthread_mutex_unlock(&scrap->mutex);
        } else {
            pthread_mutex_lock(&scrap->mutex);
            // game not found, but add it to the list with default values
            Game game;
            if (scrap->isFbNeoSid) {
                game = fbnGame;
                // pFBN custom id
                if (scrap->systemIdFbNeo == SYSTEM_ID_TG16) {
                    game.system.id = SYSTEM_ID_TG16;
                    game.system.parentId = SYSTEM_ID_PCE;
                    game.system.text = "PC Engine TurboGrafx";
                }
                scrap->gameList.games.emplace_back(game);
            } else {
                game.names.emplace_back(Game::Country::WOR, file);
                game.path = file;
                scrap->gameList.games.emplace_back(game);
            }
            if (scrap->isFbNeoSid) {
                Api::printc(COLOR_R, "[%i/%i] NOK: %s (%s) (%i)\n",
                            scrap->filesCount - filesSize, scrap->filesCount,
                            file.c_str(), fbnGame.getName().text.c_str(), gameInfo.http_error);
            } else {
                Api::printc(COLOR_R, "[%i/%i] NOK: %s (%i)\n",
                            scrap->filesCount - filesSize, scrap->filesCount, file.c_str(), gameInfo.http_error);
            }
            scrap->missList.emplace_back(game.getName().text, game.path, zipCrc, romCrc);
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
    mediasClone = args.exist("-dlmc");

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

    if (args.exist("-r")) {
        romPath = args.get("-r");
        Api::printc(COLOR_G, "Building roms list... ");
        filesList = Io::getDirList(romPath);
        filesCount = (int) filesList.size();
        Api::printc(COLOR_G, "found %zu roms\n", filesCount);
        if (filesList.empty()) {
            Api::printc(COLOR_R, "ERROR: no files found in rom path\n");
            return;
        }

        parseSid(Utility::parseInt(args.get("-sid")));
        SystemList::System system = systemList.findById(std::to_string(systemId));
        Api::printc(COLOR_G, "Scrapping system '%s', let's go!\n\n", system.names.eu.c_str());

        if (args.exist("-dlm")) {
            Io::makedir(romPath + "/media");
        }

        pthread_mutex_init(&mutex, nullptr);
        int maxThreads = user.getMaxThreads();

        for (int i = 0; i < maxThreads; i++) {
            // yes, there's a minor memory leak there...
            int *tid = (int *) malloc(sizeof(*tid));
            *tid = i;
            pthread_create(&threads[i], nullptr, scrap_thread, (void *) tid);
        }

        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threads[i], nullptr);
        }

        pthread_mutex_destroy(&mutex);

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
            for (const auto &mediaType : mediasGameList.medias) {
                if (args.exist(mediaType.nameShort)) {
                    mediaList.emplace_back(mediaType.nameShort);
                }
            }
            gameList.save(romPath + "/gamelist_pemu.xml", lang, GameList::Format::ScreenScrapper, mediaList);
        }
        Api::printc(COLOR_G, "\nAll Done... ");
        Api::printc(COLOR_G, "found %zu/%i games\n", gameList.games.size() - missList.size(), filesCount);
        if (!missList.empty()) {
            Api::printc(COLOR_O, "\n%zu game(s) not found:\n", missList.size());
            hashwrapper *md5Wrapper = new md5wrapper();
            hashwrapper *sha1Wrapper = new sha1wrapper();
            for (const auto &miss : missList) {
                std::string missInfo = Utility::getZipInfoStr(md5Wrapper, sha1Wrapper, romPath + "/", miss.path);
                Api::printc(COLOR_R, "%s\n", missInfo.c_str());
            }
            delete (md5Wrapper);
            delete (sha1Wrapper);
        }
        printf("\n");
    } else if (args.exist("-ml")) {
        Api::printc(COLOR_G, "\nAvailable screenscraper medias types:\n\n");
        for (const auto &media : mediasGameList.medias) {
            Api::printc(COLOR_G, "\t%s (type: %s, category: %s)\n",
                        media.nameShort.c_str(), media.type.c_str(), media.category.c_str());
        }
        printf("\n");
    } else if (args.exist("-sl")) {
        Api::printc(COLOR_G, "\nAvailable screenscraper systems:\n\n");
        for (const auto &system : systemList.systems) {
            Api::printc(COLOR_G, "\t%s (id: %s, company: %s, type: %s)\n",
                        system.names.eu.c_str(), system.id.c_str(), system.company.c_str(), system.type.c_str());
        }
        printf("\n");
    } else if (args.exist("-zi")) {
        const std::string fullPath = args.get("-zi");
        size_t sep = fullPath.rfind('/');
        if (sep != std::string::npos) {
            hashwrapper *md5Wrapper = new md5wrapper();
            hashwrapper *sha1Wrapper = new sha1wrapper();
            std::string zipInfo = Utility::getZipInfoStr(md5Wrapper, sha1Wrapper,
                                                         fullPath.substr(0, sep),
                                                         fullPath.substr(sep + 1, fullPath.size()));
            Api::printc(COLOR_R, "%s\n", zipInfo.c_str());
            delete (md5Wrapper);
            delete (sha1Wrapper);
        }
    } else {
        printf("\n");
        printf("usage: sscrap -u <screenscraper_user> -p <screenscraper_password> [options] [options_es]\n");
        printf("\n\toptions:\n");
        printf("\t\t-d                             enable debug output\n");
        printf("\t\t-sl                            list available screenscraper systems and exit\n");
        printf("\t\t-ml                            list available screenscraper medias types and exit\n");
        printf("\t\t-zi <rom_path>                 show zip information (size, crc, md5, sha1) and exit\n");
        printf("\t\t-r <roms_path>                 path to roms files to scrap\n");
        printf("\t\t-sid <system_id>               screenscraper system id to scrap\n");
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
