//
// Created by cpasjuste on 29/03/19.
//

#include <unistd.h>
#include <minizip/unzip.h>

#include "ss_api.h"
#include "ss_io.h"
#include "ss_gamelist.h"
#include "scrap.h"
#include "args.h"
#include "utility.h"

using namespace ss_api;

static Scrap *scrap;
static int retry_delay = 10;

void fixFbaClone(Game *game, const GameList &fbnGameList) {

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

static std::vector<Api::MediaType> mediaTypeRetry(int tid, const std::string &ssid, const std::string &sspassword) {

    std::vector<Api::MediaType> mediaTypes = Api::mediaTypes(ssid, sspassword);
    // 429 = maximum requests per minute reached, 28 timeout
    if (!mediaTypes.empty()) {
        while (mediaTypes.at(0).http_error == 429 || mediaTypes.at(0).http_error == 28) {
            pthread_mutex_lock(&scrap->mutex);
            if (mediaTypes.at(0).http_error == 429) {
                fprintf(stderr,
                        KYEL "NOK: thread[%i] => maximum requests per minute reached... retrying in %i seconds\n" KRAS,
                        tid, retry_delay);
            } else {
                fprintf(stderr,
                        KYEL "NOK: thread[%i] => timeout reached... retrying in %i seconds\n" KRAS,
                        tid, retry_delay);
            }
            pthread_mutex_unlock(&scrap->mutex);
            sleep(retry_delay);
            mediaTypes = Api::mediaTypes(ssid, sspassword);
        }
    }

    return mediaTypes;
}

static Api::GameSearch gameSearchRetry(int tid, const std::string &recherche, const std::string &systemeid,
                                       const std::string &ssid, const std::string &sspassword) {

    Api::GameSearch search = Api::gameSearch(recherche, systemeid, ssid, sspassword);
    // 429 = maximum requests per minute reached, 28 timeout
    while (search.http_error == 429 || search.http_error == 28) {
        pthread_mutex_lock(&scrap->mutex);
        if (search.http_error == 429) {
            fprintf(stderr,
                    KYEL "NOK: thread[%i] => maximum requests per minute reached... retrying in %i seconds\n" KRAS,
                    tid, retry_delay);
        } else {
            fprintf(stderr,
                    KYEL "NOK: thread[%i] => timeout reached... retrying in %i seconds\n" KRAS,
                    tid, retry_delay);
        }
        pthread_mutex_unlock(&scrap->mutex);
        sleep(retry_delay);
        search = Api::gameSearch(recherche, systemeid, ssid, sspassword);
    }

    return search;
}

static Api::GameInfo gameInfoRetry(int tid, const std::string &crc, const std::string &md5, const std::string &sha1,
                                   const std::string &systemeid, const std::string &romtype, const std::string &romnom,
                                   const std::string &romtaille, const std::string &gameid,
                                   const std::string &ssid, const std::string &sspassword) {

    Api::GameInfo gameInfo = Api::gameInfo(crc, md5, sha1, systemeid, romtype,
                                           romnom, romtaille, gameid, ssid, sspassword);
    // 429 = maximum requests per minute reached, 28 timeout
    while (gameInfo.http_error == 429 || gameInfo.http_error == 28) {
        pthread_mutex_lock(&scrap->mutex);
        if (gameInfo.http_error == 429) {
            fprintf(stderr,
                    KYEL "NOK: thread[%i] => maximum requests per minute reached... retrying in %i seconds\n" KRAS,
                    tid, retry_delay);
        } else {
            fprintf(stderr,
                    KYEL "NOK: thread[%i] => timeout reached... retrying in %i seconds\n" KRAS,
                    tid, retry_delay);
        }
        pthread_mutex_unlock(&scrap->mutex);
        sleep(retry_delay);
        gameInfo = Api::gameInfo(crc, md5, sha1, systemeid, romtype,
                                 romnom, romtaille, gameid, ssid, sspassword);
    }

    return gameInfo;
}

static void *scrap_thread(void *ptr) {

    int tid = *((int *) ptr);
    Game fbaGame;
    GameList fbaGameList;
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
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Arcade only).dat");
        } else if (id == "750") {
            // colecovision
            id = "48";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ColecoVision only).dat");
        } else if (id == "751") {
            // game gear
            id = "21";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Game Gear only).dat");
        } else if (id == "752") {
            // master system
            id = "2";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Master System only).dat");
        } else if (id == "753") {
            // megadrive
            id = "1";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Megadrive only).dat");
        } else if (id == "754") {
            // msx
            id = "113";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, MSX 1 Games only).dat");
        } else if (id == "755") {
            // pc engine
            id = "31";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, PC-Engine only).dat");
        } else if (id == "756") {
            // sega sg-1000
            id = "109";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Sega SG-1000 only).dat");
        } else if (id == "757") {
            // super grafx
            id = "105";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, SuprGrafx only).dat");
        } else if (id == "758") {
            // turbo grafx
            id = "31";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, TurboGrafx16 only).dat");
        } else if (id == "759") {
            // zx spectrum
            id = "76";
            fbaGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ZX Spectrum Games only).dat");
        }
    }

    // TODO: handle http error 430 (quota exceeded)
    while (!scrap->filesList.empty()) {

        pthread_mutex_lock(&scrap->mutex);
        std::string file = scrap->filesList.at(0);
        scrap->filesList.erase(scrap->filesList.begin());
        pthread_mutex_unlock(&scrap->mutex);

        Api::GameInfo gameInfo;
        std::string searchType = "None";

        // first, search by zip crc if not mame / fbneo arcade
        if (id != "75") {
            std::string path = scrap->romPath + "/" + file;
            std::string crc = Utility::getZipCrc(path);
            if (!crc.empty()) {
                gameInfo = gameInfoRetry(tid, crc, "", "", id, "",
                                         file, "", "", scrap->user, scrap->pwd);
                if (gameInfo.http_error == 0) {
                    searchType = "zip_crc";
                }
            }
            SS_PRINT("zip crc: %s (%s), res = %i\n", file.c_str(), crc.c_str(), gameInfo.http_error);
        }

        // next, try by rom crc if not mame / fbneo arcade
        if (gameInfo.http_error != 0 && id != "75") {
            std::string path = scrap->romPath + "/" + file;
            std::string crc = Utility::getRomCrc(path);
            if (!crc.empty()) {
                gameInfo = gameInfoRetry(tid, crc, "", "", id, "",
                                         file, "", "", scrap->user, scrap->pwd);
                if (gameInfo.http_error == 0) {
                    searchType = "rom_crc";
                }
            }
            SS_PRINT("rom crc: %s (%s), res = %i\n", file.c_str(), crc.c_str(), gameInfo.http_error);
        }

        // fbneo consoles zip names doesn't match standard consoles zip names
        if (isFbNeoSystem && id != "75") {
            fbaGame = fbaGameList.findByPath(file);
        }

        // now try with zip name
        if (gameInfo.http_error != 0) {
            std::string romType = scrap->args.exist("-romtype") ? scrap->args.get("-romtype") : "rom";
            std::string name = (isFbNeoSystem && id != "75") ? fbaGame.getName().text + ".zip" : file;
            gameInfo = gameInfoRetry(tid, "", "", "", id, romType,
                                     name, "", "", scrap->user, scrap->pwd);
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
            std::string name = isFbNeoSystem ? fbaGame.getName().text : file;
            Api::GameSearch search = gameSearchRetry(tid, name, id, scrap->user, scrap->pwd);
            SS_PRINT("search name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
            if (!search.games.empty()) {
                auto game = std::find_if(search.games.begin(), search.games.end(), [id](const Game &game) {
                    return game.system.id == id;
                });
                if (game != search.games.end()) {
                    gameInfo = gameInfoRetry(tid, "", "", "", "", "",
                                             file, "", (*game).id, scrap->user, scrap->pwd);
                    if (gameInfo.http_error == 0) {
                        searchType = "search";
                    }
                    SS_PRINT("search name (romid): %s, res = %i\n", name.c_str(), gameInfo.http_error);
                }
            }
        }

        if (isFbNeoSystem) {
            fixFbaClone(&gameInfo.game, fbaGameList);
        }

        if (gameInfo.http_error == 0) {
            if (scrap->args.exist("-medias") && (!scrap->mediasClone && !gameInfo.game.isClone())) {
                for (const auto &mediaType : scrap->mediaTypes) {
                    if (!scrap->args.exist(mediaType.name)) {
                        continue;
                    }
                    Game::Media media = gameInfo.game.getMedia(mediaType.name, Game::Country::SS);
                    if (media.url.empty()) {
                        continue;
                    }
                    std::string name = file.substr(0, file.find_last_of('.') + 1) + media.format;
                    std::string path = scrap->romPath + "/media/" + media.type + "/";
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
                                tid, retry_delay);
                        pthread_mutex_unlock(&scrap->mutex);
                        sleep(retry_delay);
                        res = media.download(path);
                    }
                }
            }

            pthread_mutex_lock(&scrap->mutex);
            printf(KGRE "[%i/%i] OK: %s => %s (%s) (%s)\n" KRAS,
                   scrap->filesCount - (int) scrap->filesList.size(), scrap->filesCount,
                   file.c_str(), gameInfo.game.getName().text.c_str(),
                   gameInfo.game.system.text.c_str(), searchType.c_str());
            scrap->gameList.games.emplace_back(gameInfo.game);
            pthread_mutex_unlock(&scrap->mutex);
        } else {
            pthread_mutex_lock(&scrap->mutex);
            // game not found, but add it to the list with default values
            Game game;
            game.names.emplace_back(Api::toString(Game::Country::WOR), file);
            game.path = file;
            scrap->gameList.games.emplace_back(game);
            fprintf(stderr, KRED "[%i/%i] NOK: %s (%i)\n" KRAS,
                    scrap->filesCount - (int) scrap->filesList.size(), scrap->filesCount,
                    file.c_str(), gameInfo.http_error);
            scrap->missList.emplace_back(file);
            pthread_mutex_unlock(&scrap->mutex);
        }
    }

    return nullptr;
}

Scrap::Scrap(const ArgumentParser &parser) {

    args = parser;
    user = args.get("-user");
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

    int thread_count = 1;

    if (args.exist("-gameinfo")) {
        if (args.exist("-romspath")) {
            romPath = args.get("-romspath");
            filesList = Io::getDirList(romPath);
            filesCount = filesList.size();
            mediaTypes = mediaTypeRetry(-1, user, pwd);
            if (filesList.empty()) {
                fprintf(stderr, KRED "ERROR: no files found in rom path\n" KRAS);
                return;
            }

            if (args.exist("-medias")) {
                Io::makedir(romPath + "/media");
            }

            if (args.exist("-threads")) {
                thread_count = (int) std::strtol(args.get("-threads").c_str(), nullptr, 10);
            }

            for (int i = 0; i < thread_count; i++) {
                // yes, there's a minor memory leak there...
                int *tid = (int *) malloc(sizeof(*tid));
                *tid = i;
                pthread_create(&threads[i], nullptr, scrap_thread, (void *) tid);
            }

            for (int i = 0; i < thread_count; i++) {
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
            Api::GameInfo gameInfo = Api::gameInfo(args.get("-crc"), args.get("-md5"), args.get("-sha1"),
                                                   args.get("-systemid"), scrap->args.get("-romtype"),
                                                   args.get("-romname"), args.get("-romsize"),
                                                   args.get("-gameid"), user, pwd);
            printf("\n===================================\n");
            printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
                   gameInfo.ssuser.id.c_str(), gameInfo.ssuser.maxrequestsperday.c_str(),
                   gameInfo.ssuser.maxthreads.c_str());
            if (!gameInfo.game.id.empty()) {
                Utility::printGame(gameInfo.game);
            } else {
                printf("gameInfo: game not found\n");
            }
        }
    } else if (args.exist("-gamesearch")) {
        Api::GameSearch search = Api::gameSearch(args.get("-gamename"), args.get("-systemid"), user, pwd);
        printf("\n===================================\n");
        printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
               search.ssuser.id.c_str(), search.ssuser.maxrequestsperday.c_str(),
               search.ssuser.maxthreads.c_str());
        printf("games found: %li\n", search.games.size());
        for (auto &game : search.games) {
            Utility::printGame(game);
        }
    } else if (args.exist("-mediatypes")) {
        mediaTypes = Api::mediaTypes(user, pwd);
        printf("\nAvailable screenscraper medias:\n");
        for (const auto &mediaType : mediaTypes) {
            printf("\t%s\n", mediaType.name.c_str());
        }
    } else {
        printf("usage: sscrap -user <screenscraper_user> -password <screenscraper_password> [options]\n");
        printf("\toptions:\n");
        printf("\t\t-threads <threads count>\n");
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
    ss_debug = true;
#endif

    scrap = new Scrap(args);
    scrap->run();

    return 0;
}
