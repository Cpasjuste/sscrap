//
// Created by cpasjuste on 29/03/19.
//

#include <unistd.h>
#include "ss_api.h"
#include "ss_io.h"
#include "ss_gamelist.h"
#include "scrap.h"
#include "args.h"

using namespace ss_api;

static Scrap *scrap;

void printGame(const Game &game) {

    printf("\n===================================\n");
    Game::Name name = game.getName(Game::Country::SS);
    printf("nom (%s): %s (alternatives: %li)\n", name.country.c_str(), name.text.c_str(), game.names.size() - 1);
    printf("available: %i\n", (int) game.available);
    printf("path: %s\n", game.path.c_str());
    for (auto &country : game.countries) {
        printf("country: %s\n", country.c_str());
    }
    printf("id: %s\n", game.id.c_str());
    printf("cloneof: %s\n", game.cloneof.c_str());
    printf("system: %s (id: %s)\n", game.system.text.c_str(), game.system.id.c_str());
    printf("editor: %s (id: %s)\n", game.editor.text.c_str(), game.editor.id.c_str());
    printf("developer: %s (id: %s)\n", game.developer.text.c_str(), game.developer.id.c_str());
    printf("players: %s\n", game.players.c_str());
    printf("rating: %s\n", game.rating.c_str());
    printf("topstaff: %s\n", game.topstaff.c_str());
    printf("rotation: %s\n", game.rotation.c_str());
    printf("resolution: %s\n", game.resolution.c_str());
    printf("inputs: %s\n", game.inputs.c_str());
    printf("colors: %s\n", game.colors.c_str());
    Game::Synopsis synopsis = game.getSynopsis(Game::Language::EN);
    printf("synopsis (%s): %s\n", synopsis.language.c_str(), synopsis.text.c_str());
    Game::Date date = game.getDate(Game::Country::WOR);
    printf("date (%s): %s\n", date.country.c_str(), date.text.c_str());
    Game::Genre genre = game.getGenre(Game::Language::EN);
    printf("genre (%s): %s\n", genre.language.c_str(), genre.text.c_str());
    // print some medias
    Game::Media media = game.getMedia("sstitle", Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    media = game.getMedia("ss", Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    media = game.getMedia("mixrbv2", Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
}

void fixFbaClone(Game *game, const GameList &fbnGameList, const std::string &systemId) {

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
    int retry_delay = 10;
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

    // TODO: handle http error 400 (quota exceeded)

    while (!scrap->filesList.empty()) {

        pthread_mutex_lock(&scrap->mutex);
        std::string file = scrap->filesList.at(0);
        scrap->filesList.erase(scrap->filesList.begin());
        pthread_mutex_unlock(&scrap->mutex);

        if (isFbNeoSystem && id != "75") {
            fbaGame = fbaGameList.findByPath(file);
            if (!fbaGame.getName().text.empty()) {
                file = fbaGame.getName().text + ".zip";
            }
        }

        std::string romType = scrap->args.exist("-romtype") ? scrap->args.get("-romtype") : "rom";
        Api::GameInfo gameInfo = Api::gameInfo("", "", "", id, romType,
                                               file, "", "", scrap->user, scrap->pwd);
        // 429 = maximum requests per minute reached
        while (gameInfo.http_error == 429) {
            pthread_mutex_lock(&scrap->mutex);
            fprintf(stderr,
                    KYEL "NOK: thread[%i] => maximum requests per minute reached... retrying in %i seconds\n" KRAS,
                    tid, retry_delay);
            pthread_mutex_unlock(&scrap->mutex);
            sleep(retry_delay);
            gameInfo = Api::gameInfo("", "", "", id, romType,
                                     file, "", "", scrap->user, scrap->pwd);
        }

        // 28 = CURLE_OPERATION_TIMEDOUT
        while (gameInfo.http_error == 28) {
            pthread_mutex_lock(&scrap->mutex);
            fprintf(stderr,
                    KYEL "NOK: thread[%i] => timeout reached... retrying in %i seconds\n" KRAS,
                    tid, retry_delay);
            pthread_mutex_unlock(&scrap->mutex);
            sleep(retry_delay);
            gameInfo = Api::gameInfo("", "", "", id, romType,
                                     file, "", "", scrap->user, scrap->pwd);
        }

        if (isFbNeoSystem) {
            // restore correct rom path
            if (id != "75") {
                gameInfo.game.path = file = fbaGame.path;
            }
            fixFbaClone(&gameInfo.game, fbaGameList, id);
        }

        if (gameInfo.http_error != 404) {
            if (scrap->args.exist("-medias") && (!scrap->mediasClone && !gameInfo.game.isClone())) {
                for (const auto &mediaType : scrap->mediaTypes) {
                    if (scrap->args.exist(mediaType.name)) {
                        Game::Media media = gameInfo.game.getMedia(mediaType.name, Game::Country::SS);
                        if (!media.url.empty()) {
                            std::string name = gameInfo.game.path.substr(0, gameInfo.game.path.find_last_of('.') + 1)
                                               + media.format;
                            std::string path = scrap->romPath + "/media/" + media.type + "/";
                            if (!Io::exist(path)) {
                                Io::makedir(path);
                            }
                            path += name;
                            if (!Io::exist(path)) {
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
                    }
                }
            }

            pthread_mutex_lock(&scrap->mutex);
            printf(KGRE "OK: %s => %s (%s)\n" KRAS,
                   file.c_str(), gameInfo.game.getName().text.c_str(), gameInfo.game.system.text.c_str());
            scrap->gameList.games.emplace_back(gameInfo.game);
            pthread_mutex_unlock(&scrap->mutex);
        } else {
            pthread_mutex_lock(&scrap->mutex);
            // game not found, but add it to the list with default values
            Game game;
            game.names.emplace_back(Api::toString(Game::Country::WOR), file);
            game.path = file;
            scrap->gameList.games.emplace_back(game);
            fprintf(stderr, KRED "NOK: %s (%i)\n" KRAS, file.c_str(), gameInfo.http_error);
            scrap->missList.emplace_back(file);
            pthread_mutex_unlock(&scrap->mutex);
            break;
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

#if 1
    int thread_count = 1;

    if (args.exist("-gameinfo")) {
        if (args.exist("-romspath")) {
            romPath = args.get("-romspath");
            filesList = Io::getDirList(romPath);
            filesCount = filesList.size();
            mediaTypes = Api::mediaTypes(user, pwd);
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
            std::string romType = scrap->args.exist("-romtype") ? scrap->args.get("-romtype") : "rom";
            Api::GameInfo gameInfo = Api::gameInfo(args.get("-crc"), args.get("-md5"), args.get("-sha1"),
                                                   args.get("-systemid"), romType, args.get("-romname"),
                                                   args.get("-romsize"), args.get("-gameid"), user, pwd);
            printf("\n===================================\n");
            printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
                   gameInfo.ssuser.id.c_str(), gameInfo.ssuser.maxrequestsperday.c_str(),
                   gameInfo.ssuser.maxthreads.c_str());
            if (!gameInfo.game.id.empty()) {
                printGame(gameInfo.game);
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
            printGame(game);
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
#endif

#if 0
    gameList = GameList("../databases/FinalBurn Neo (ClrMame Pro XML, Arcade only).dat");
    gameList.append("../databases/FinalBurn Neo (ClrMame Pro XML, Megadrive only).dat");
    //if (!gameList.fixClones("fbneo.dat")) {
    //    exit(-1);
    //}
    //gameList.save("gamelist_fixed.xml",
    //              Game::Language::EN, GameList::Format::ScreenScrapper);
    //exit(0);

    printf("total games: %zu\n", gameList.games.size());
    gameList = gameList.filter();
    printf("total parents: %zu\n", gameList.games.size());

    std::vector<std::string> names;
    for (const auto &game : gameList.games) {

        std::vector<Game> clones;
        std::string name = game.getName().text;
        auto found = std::find_if(names.begin(), names.end(), [&name](const std::string &n) {
            return name == n;
        });

        if (found != names.end())
            continue;

        names.emplace_back(name);
        std::copy_if(gameList.games.begin(), gameList.games.end(), std::back_inserter(clones), [name](const Game &g) {
            return g.getName().text == name;
        });

        if (clones.size() > 1) {
            printf("%s: found %zu clones: ", name.c_str(), clones.size());
            for (const auto &g : clones) {
                printf("%s (%s) - ", g.romid.c_str(), g.path.c_str());
                //printGame(g);
            }
            printf("\n");
        }
    }
#endif
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
