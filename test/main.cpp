//
// Created by cpasjuste on 29/03/19.
//

#include <unistd.h>
#include "ss_api.h"
#include "ss_io.h"
#include "scrap.h"
#include "args.h"

using namespace ss_api;

static Scrap *scrap;

void printGame(const Game &game) {

    printf("\n===================================\n");
    Game::Name name = game.getName(Game::Country::SS);
    printf("nom (%s): %s (alternatives: %li)\n", name.country.c_str(), name.text.c_str(), game.names.size() - 1);
    printf("available: %i\n", game.available);
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
    Game::Media media = game.getMedia(Game::Media::Type::SSTitle, Game::Country::WOR);
    //media.download("cache/" + game.id + "_" + media.type + "_" + media.country + "." + media.format);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    media = game.getMedia(Game::Media::Type::SS, Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    media = game.getMedia(Game::Media::Type::Mixrbv2, Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
}

static void *scrap_thread(void *ptr) {

    int tid = *((int *) ptr);
    int retry_delay = 10;

    // TODO: handle http error 400 (quota exceeded)

    while (!scrap->fileList.empty()) {

        pthread_mutex_lock(&scrap->mutex);
        std::string file = scrap->fileList.at(0);
        scrap->fileList.erase(scrap->fileList.begin());
        pthread_mutex_unlock(&scrap->mutex);

        Api::GameInfo gameInfo = Api::gameInfo("", "", "",
                                               scrap->args.get("-systemid"), scrap->args.get("-romtype"),
                                               file, "", "", scrap->user, scrap->pwd);
        // 429 = maximum requests per minute reached
        while (gameInfo.http_error == 429) {
            pthread_mutex_lock(&scrap->mutex);
            fprintf(stderr,
                    KYEL "NOK: thread[%i] => maximum requests per minute reached... retrying in %i seconds\n" KRAS,
                    tid, retry_delay);
            pthread_mutex_unlock(&scrap->mutex);
            sleep(retry_delay);
            gameInfo = Api::gameInfo("", "", "",
                                     scrap->args.get("-systemid"), scrap->args.get("-romtype"),
                                     file, "", "", scrap->user, scrap->pwd);
        }

        if (!gameInfo.game.id.empty()) {
            if (scrap->args.exist("-medias") && (!scrap->mediasClone && gameInfo.game.cloneof == "0")) {
                Game::Media media = gameInfo.game.getMedia(Game::Media::Type::SS, Game::Country::SS);
                if (!media.url.empty()) {
                    std::string name = gameInfo.game.path.substr(0, gameInfo.game.path.find_last_of('.') + 1);
                    std::string path = scrap->romPath + "/media/images/" + name + media.format;
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
                media = gameInfo.game.getMedia(Game::Media::Type::Box3D, Game::Country::SS);
                if (!media.url.empty()) {
                    std::string name = gameInfo.game.path.substr(0, gameInfo.game.path.find_last_of('.') + 1);
                    std::string path = scrap->romPath + "/media/box3d/" + name + media.format;
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
                media = gameInfo.game.getMedia(Game::Media::Type::Video, Game::Country::ALL);
                if (!media.url.empty()) {
                    std::string name = gameInfo.game.path.substr(0, gameInfo.game.path.find_last_of('.') + 1);
                    std::string path = scrap->romPath + "/media/videos/" + name + media.format;
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
        if (args.exist("-rompath")) {
            romPath = args.get("-rompath");
            fileList = Io::getDirList(romPath);
            if (fileList.empty()) {
                fprintf(stderr, KRED "ERROR: no files found in rom path\n" KRAS);
                return;
            }

            if (args.exist("-medias")) {
                Io::makedir(romPath + "/media");
                Io::makedir(romPath + "/media/box3d");
                Io::makedir(romPath + "/media/images");
                Io::makedir(romPath + "/media/videos");
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

            gameList.roms_count = gameList.games.size();
            if (!gameList.games.empty() && args.exist("-savexml")) {
                gameList.save(romPath + "/gamelist.xml");
            }

            printf(KGRE "\n==========\nALL DONE\n==========\n" KRAS);
            printf(KGRE "found %i games, %zu was not scrapped:" KRAS, gameList.roms_count, missList.size());
            if (!missList.empty()) {
                printf(KGRE ", missing games:\n" KRAS);
                for (const auto &file : missList) {
                    printf(KRED "%s, " KRAS, file.c_str());
                }
            }
            printf("\n");
        } else {
            Api::GameInfo gameInfo = Api::gameInfo(args.get("-crc"), args.get("-md5"), args.get("-sha1"),
                                                   args.get("-systemid"), args.get("-romtype"), args.get("-romname"),
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
    } else {
        fprintf(stderr, KRED "TODO: PRINT HELP\n" KRAS);
    }
#endif

#if 0
    gameList = Api::gameList("gamelist.xml");
    //Api::gameListFixClones(&gameList, "fbneo.dat");
    //gameList.save("gamelist_fixed.xml");

    printf("total games: %zu\n", gameList.games.size());
    std::vector<Game> games = Api::gameListFilter(gameList.games);
    printf("total parents: %zu\n", games.size());

    std::vector<std::string> names;
    for (const auto &game : games) {

        std::vector<Game> clones;
        std::string name = game.getName().text;
        auto found = std::find_if(names.begin(), names.end(), [&name](const std::string &n) {
            return name == n;
        });

        if(found != names.end())
            continue;

        names.emplace_back(name);
        std::copy_if(games.begin(), games.end(), std::back_inserter(clones), [name](const Game &g) {
            return g.getName().text == name;
        });

        if (clones.size() > 1) {
            printf("%s: found %zu clones: ", name.c_str(), clones.size());
            for (const auto &g : clones) {
                printf("%s (%s) - ", g.romid.c_str(), g.path.c_str());
                //printGame(g);
            }
            printf("\n");
            //break;
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
