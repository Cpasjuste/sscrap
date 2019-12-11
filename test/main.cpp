//
// Created by cpasjuste on 29/03/19.
//
#include <unistd.h>
#include "ss_api.h"
#include "ss_io.h"
#include "ss_gamelist.h"
#include "scrap.h"
#include "args.h"
#include "utility.h"

using namespace ss_api;

static Scrap *scrap;
static int retryDelay = 10;

void fixFbnClone(Game *game, const GameList &fbnGameList) {

    // screenscraper game "cloneof" is set, continue
    if (game->isClone()) {
        return;
    }

    // mame/fbneo: search fbn list for zip name (path), as game name may differ
    const std::string zipName = game->path;
    auto fbaGame = std::find_if(fbnGameList.games.begin(), fbnGameList.games.end(), [zipName](const Game &g) {
        return zipName == g.path && g.isClone();
    });
    // screenscraper game not found in fbneo dat, or is not a clone continue...
    if (fbaGame == fbnGameList.games.end()) {
        return;
    }
    // fix screenscraper cloneof !
    game->cloneof = (*fbaGame).cloneof + ".zip";
}

static void *scrap_thread(void *ptr) {

    int tid = *((int *) ptr);
    Game fbnGame;
    GameList fbnGameList;
    bool isFbNeoSystem = false;

    // if a custom sscrap custom id is set (fbneo console games),
    // we need to use "FinalBurn Neo" databases "description" as rom name
    // and map to correct screenscraper systemid
    std::string id = scrap->args.get("-systemid");
    if (id == "75" || id == "750" || id == "751" || id == "752" || id == "753"
        || id == "754" || id == "755" || id == "756" || id == "757" || id == "758" || id == "759") {
        isFbNeoSystem = true;
        if (id == "75") {
            // mame/fbneo
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Arcade only).dat");
        } else if (id == "750") {
            // colecovision
            id = "48";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ColecoVision only).dat");
        } else if (id == "751") {
            // game gear
            id = "21";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Game Gear only).dat");
        } else if (id == "752") {
            // master system
            id = "2";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Master System only).dat");
        } else if (id == "753") {
            // megadrive
            id = "1";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Megadrive only).dat");
        } else if (id == "754") {
            // msx
            id = "113";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, MSX 1 Games only).dat");
        } else if (id == "755") {
            // pc engine
            id = "31";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, PC-Engine only).dat");
        } else if (id == "756") {
            // sega sg-1000
            id = "109";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Sega SG-1000 only).dat");
        } else if (id == "757") {
            // super grafx
            id = "105";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, SuprGrafx only).dat");
        } else if (id == "758") {
            // turbo grafx
            id = "31";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, TurboGrafx16 only).dat");
        } else if (id == "759") {
            // zx spectrum
            id = "76";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ZX Spectrum Games only).dat");
        }
    }

    while (!scrap->filesList.empty()) {

        pthread_mutex_lock(&scrap->mutex);
        std::string file = scrap->filesList.at(0);
        scrap->filesList.erase(scrap->filesList.begin());
        int filesSize = scrap->filesList.size();
        pthread_mutex_unlock(&scrap->mutex);

        GameInfo gameInfo;
        std::string searchType = "None";

        // first, search by zip crc
        std::string path = scrap->romPath + "/" + file;
        std::string crc = Utility::getZipCrc(path);
        if (!crc.empty()) {
            gameInfo = GameInfo(crc, "", "", id, "", file,
                                "", "", scrap->usr, scrap->pwd, retryDelay);
            if (gameInfo.http_error == 0) {
                searchType = "zip_crc";
            } else if (gameInfo.http_error == 430) {
                fprintf(stderr, KYEL "NOK: thread[%i] => Quota reached for today... "
                                "See \' https://www.screenscraper.fr/\' if you want to support "
                                "screenscraper and maximise your quota!\n" KRAS, tid);
                break;
            }
        }
        SS_PRINT("zip crc: %s (%s), res = %i\n", file.c_str(), crc.c_str(), gameInfo.http_error);

        // next, try by rom crc if not mame / fbneo arcade (multiple roms in zip...)
        if (gameInfo.http_error != 0 && id != "75") {
            path = scrap->romPath + "/" + file;
            crc = Utility::getRomCrc(path);
            if (!crc.empty()) {
                gameInfo = GameInfo(crc, "", "", id, "", file,
                                    "", "", scrap->usr, scrap->pwd, retryDelay);
                if (gameInfo.http_error == 0) {
                    searchType = "rom_crc";
                } else if (gameInfo.http_error == 430) {
                    fprintf(stderr, KYEL "NOK: thread[%i] => Quota reached for today... "
                                    "See \' https://www.screenscraper.fr/\' if you want to support "
                                    "screenscraper and maximise your quota!\n" KRAS, tid);
                    break;
                }
            }
            SS_PRINT("rom crc: %s (%s), res = %i\n", file.c_str(), crc.c_str(), gameInfo.http_error);
        }

        // fbneo consoles zip names doesn't match standard consoles zip names
        // this will also help fbneo arcade games if not found by zip name
        if (isFbNeoSystem) {
            fbnGame = fbnGameList.findByPath(file);
        }

        // now try with zip name
        if (gameInfo.http_error != 0) {
            std::string romType = scrap->args.exist("-romtype") ? scrap->args.get("-romtype") : "rom";
            std::string name = (isFbNeoSystem && id != "75") ? fbnGame.getName().text + ".zip" : file;
            gameInfo = GameInfo("", "", "", id, romType, name,
                                "", "", scrap->usr, scrap->pwd, retryDelay);
            if (gameInfo.http_error == 0) {
                searchType = "zip_name";
            }
            if (isFbNeoSystem && id != "75") {
                gameInfo.game.path = file;
            }
            SS_PRINT("zip name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
        }

        // finally, try a game search (jeuRecherche)
        if (gameInfo.http_error != 0) {
            // the rom is not know by screenscraper, try to find the game with a game search (jeuRecherche)
            std::string name = isFbNeoSystem ? fbnGame.getName().text : file;
            GameSearch search = GameSearch(name, id, scrap->usr, scrap->pwd, retryDelay);
            SS_PRINT("search name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
            if (!search.games.empty()) {
                auto game = std::find_if(search.games.begin(), search.games.end(), [id](const Game &game) {
                    return game.system.id == id;
                });
                if (game != search.games.end()) {
                    gameInfo = GameInfo("", "", "", "", "", file,
                                        "", (*game).id, scrap->usr, scrap->pwd, retryDelay);
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
                    search = GameSearch(name, id, scrap->usr, scrap->pwd, retryDelay);
                    SS_PRINT("search name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
                    if (!search.games.empty()) {
                        auto game = std::find_if(search.games.begin(), search.games.end(), [id](const Game &game) {
                            return game.system.id == id;
                        });
                        if (game != search.games.end()) {
                            gameInfo = GameInfo("", "", "", "", "", file,
                                                "", (*game).id, scrap->usr, scrap->pwd, retryDelay);
                            if (gameInfo.http_error == 0) {
                                searchType = "search";
                            }
                            SS_PRINT("search name (romid): %s, res = %i\n", name.c_str(), gameInfo.http_error);
                        }
                    }
                }
            }
        }

        if (isFbNeoSystem) {
            fixFbnClone(&gameInfo.game, fbnGameList);
        }

        if (gameInfo.http_error == 0) {
            if (scrap->args.exist("-medias") && (!scrap->mediasClone && !gameInfo.game.isClone())) {
                for (const auto &mediaType : scrap->mediasGameList.medias) {
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
                    int res = media.download(path);
                    while (res == 429) {
                        pthread_mutex_lock(&scrap->mutex);
                        fprintf(stderr,
                                KYEL "NOK: thread[%i] => maximum requests per minute reached... retrying in %i seconds\n" KRAS,
                                tid, retryDelay);
                        pthread_mutex_unlock(&scrap->mutex);
                        sleep(retryDelay);
                        res = media.download(path);
                    }
                }
            }

            pthread_mutex_lock(&scrap->mutex);
            printf(KGRE "[%i/%i] OK: %s => %s (%s) (%s)\n" KRAS,
                   scrap->filesCount - filesSize, scrap->filesCount,
                   file.c_str(), gameInfo.game.getName().text.c_str(),
                   gameInfo.game.system.text.c_str(), searchType.c_str());
            scrap->gameList.games.emplace_back(gameInfo.game);
            pthread_mutex_unlock(&scrap->mutex);
        } else {
            pthread_mutex_lock(&scrap->mutex);
            // game not found, but add it to the list with default values
            if (isFbNeoSystem) {
                scrap->gameList.games.emplace_back(fbnGame);
            } else {
                Game game;
                game.names.emplace_back(Api::toString(Game::Country::WOR), file);
                game.path = file;
                scrap->gameList.games.emplace_back(game);
            }
            fprintf(stderr, KRED "[%i/%i] NOK: %s (%i)\n" KRAS,
                    scrap->filesCount - filesSize, scrap->filesCount,
                    isFbNeoSystem ? fbnGame.getName().text.c_str() : file.c_str(), gameInfo.http_error);
            scrap->missList.emplace_back(file);
            pthread_mutex_unlock(&scrap->mutex);
        }
    }

    return nullptr;
}

Scrap::Scrap(const ArgumentParser &parser) {

    args = parser;
    usr = args.get("-user");
    pwd = args.get("-password");
    mediasClone = args.exist("-mediasclone");

    if (args.exist("-language")) {
        language = Api::toLanguage(args.get("-language"));
        if (language == Game::Language::UNKNOWN) {
            fprintf(stderr, KRED "ERROR: language not found: %s, available languages: en, fr, es, pt\n" KRAS,
                    args.get("-language").c_str());
            return;
        }
        printf("language: %s\n", Api::toString(language).c_str());
    }
}

void Scrap::run() {

    if (args.exist("-gameinfo")) {
        if (args.exist("-romspath")) {
            romPath = args.get("-romspath");
            filesList = Io::getDirList(romPath);
            filesCount = filesList.size();
            if (filesList.empty()) {
                fprintf(stderr, KRED "ERROR: no files found in rom path\n" KRAS);
                return;
            }

            user = User(usr, pwd);
            mediasGameList = MediasGameList(usr, pwd, retryDelay);

            if (args.exist("-medias")) {
                Io::makedir(romPath + "/media");
            }

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

            if (!gameList.games.empty() && args.exist("-savexml")) {
                gameList.save(romPath + "/gamelist.xml");
            }
            printf(KGRE "\n==========\nALL DONE\n==========\n" KRAS);
            printf(KGRE "found %zu/%i games\n" KRAS, gameList.games.size() - missList.size(), filesCount);
            if (!missList.empty()) {
                printf(KGRE "%zu was not found:\n" KRAS, missList.size());
                for (const auto &file : missList) {
                    printf(KRED "%s, " KRAS, file.c_str());
                }
            }
            printf("\n");
        } else {
            GameInfo gameInfo = GameInfo(args.get("-crc"), args.get("-md5"), args.get("-sha1"),
                                         args.get("-systemid"), scrap->args.get("-romtype"),
                                         args.get("-romname"), args.get("-romsize"),
                                         args.get("-gameid"), usr, pwd, retryDelay);
            printf("\n===================================\n");
            printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
                   gameInfo.user.id.c_str(), gameInfo.user.maxrequestsperday.c_str(),
                   gameInfo.user.maxthreads.c_str());
            if (!gameInfo.game.id.empty()) {
                Utility::printGame(gameInfo.game);
            } else {
                printf("gameInfo: game not found\n");
            }
        }
    } else if (args.exist("-gamesearch")) {
        GameSearch search = GameSearch(args.get("-gamename"), args.get("-systemid"),
                                       usr, pwd, retryDelay);
        printf("\n===================================\n");
        printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
               search.user.id.c_str(), search.user.maxrequestsperday.c_str(),
               search.user.maxthreads.c_str());
        printf("games found: %li\n", search.games.size());
        for (auto &game : search.games) {
            Utility::printGame(game);
        }
    } else if (args.exist("-mediatypes")) {
        mediasGameList = MediasGameList(usr, pwd, retryDelay);
        printf("\nAvailable screenscraper medias:\n");
        for (const auto &media : mediasGameList.medias) {
            printf("\t%s\n", media.nameShort.c_str());
        }
    } else {
        printf("usage: sscrap -user <screenscraper_user> -password <screenscraper_password> [options]\n");
        printf("\toptions:\n");
        printf("\t\t-debug\n");
        printf("\t\t-gameinfo [gameinfo options]\n");
        printf("\t\t\tgameinfo options:\n");
        printf("\t\t\t\t-crc <rom crc>\n");
        printf("\t\t\t\t-md5 <rom md5>\n");
        printf("\t\t\t\t-sha1 <rom sha1>\n");
        printf("\t\t\t\t-systemid <system id>\n");
        printf("\t\t\t\t-romtype <rom type>\n");
        printf("\t\t\t\t-romname <rom name>\n");
        printf("\t\t\t\t-romsize <rom size>\n");
        printf("\t\t\t\t-gameid <game id>\n");
        printf("\t\t\t\t-medias\n");
        printf("\t\t\t\t-mediasclone\n");
        printf("\t\t\t\t-romspath <roms path>\n");
        printf("\t\t\t\t-savexml\n");
        printf("\t\t-gamesearch [gamesearch options]\n");
        printf("\t\t\tgamesearch options:\n");
        printf("\t\t\t\t-gamename <game name>\n");
        printf("\t\t\t\t-systemid <system id>\n");
        printf("\tsscrap customs systemid (fbneo)\n");
        printf("\t\t750: ColecoVision\n");
        printf("\t\t751: Game Gear\n");
        printf("\t\t752: Master System\n");
        printf("\t\t753: Megadrive\n");
        printf("\t\t754: MSX 1\n");
        printf("\t\t755: PC-Engine\n");
        printf("\t\t756: Sega SG-1000\n");
        printf("\t\t757: Super Grafx\n");
        printf("\t\t758: Turbo Grafx\n");
        printf("\t\t759: ZX Spectrum\n");
    }
}

int main(int argc, char **argv) {

    ArgumentParser args(argc, argv);

    // setup screenscraper api
    Api::ss_devid = SS_DEV_ID;
    Api::ss_devpassword = SS_DEV_PWD;
    Api::ss_softname = "sscrap";
    ss_debug = args.exist("-debug");
#ifndef NDEBUG
    //ss_debug = true;
#endif

    scrap = new Scrap(args);
    scrap->run();

    return 0;
}
