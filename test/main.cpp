//
// Created by cpasjuste on 29/03/19.
//

#include <unistd.h>
#include "ss_api.h"
#include "ss_io.h"
#include "scrap.h"
#include "args.h"

#define KRED "\x1B[31m"
#define KGRE "\x1B[92m"
#define KRAS "\033[0m"

using namespace ss_api;

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

    auto scrapper = (Scrap *) ptr;

    while (!scrapper->fileList.empty()) {

        pthread_mutex_lock(&scrapper->mutex);
        std::string file = scrapper->fileList.at(0);
        scrapper->fileList.erase(scrapper->fileList.begin());
        pthread_mutex_unlock(&scrapper->mutex);

        Api::GameInfo gameInfo = Api::gameInfo("", "", "",
                                               scrapper->args.get("-systemid"), scrapper->args.get("-romtype"),
                                               file, "", "", scrapper->user, scrapper->pwd);
        if (!gameInfo.game.id.empty()) {
            if (scrapper->args.exist("-dl")) {
                Game::Media media = gameInfo.game.getMedia(Game::Media::Type::SS, Game::Country::SS);
                if (!media.url.empty()) {
                    std::string name = gameInfo.game.path.substr(0, gameInfo.game.path.find_last_of('.') + 1);
                    std::string path = "media/images/" + name + media.format;
                    media.download(path);
                }
                media = gameInfo.game.getMedia(Game::Media::Type::Box3D, Game::Country::SS);
                if (!media.url.empty()) {
                    std::string name = gameInfo.game.path.substr(0, gameInfo.game.path.find_last_of('.') + 1);
                    std::string path = "media/box3d/" + name + media.format;
                    media.download(path);
                }
                media = gameInfo.game.getMedia(Game::Media::Type::Video, Game::Country::ALL);
                if (!media.url.empty()) {
                    std::string name = gameInfo.game.path.substr(0, gameInfo.game.path.find_last_of('.') + 1);
                    std::string path = "media/videos/" + name + media.format;
                    media.download(path);
                }
            }
            pthread_mutex_lock(&scrapper->mutex);
            printf(KGRE "OK: %s => %s (%s)\n" KRAS,
                   file.c_str(), gameInfo.game.getName().text.c_str(), gameInfo.game.system.text.c_str());
            scrapper->gameList.games.emplace_back(gameInfo.game);
            pthread_mutex_unlock(&scrapper->mutex);
        } else {
            pthread_mutex_lock(&scrapper->mutex);
            fprintf(stderr, KRED "NOK: %s\n" KRAS, file.c_str());
            scrapper->missList.emplace_back(file);
            pthread_mutex_unlock(&scrapper->mutex);
        };
    }

    return nullptr;
}

Scrap::Scrap(const ArgumentParser &parser) {

    args = parser;
    user = args.get("-user");
    pwd = args.get("-password");
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
        if (args.exist("-rompath")) {
            fileList = Io::getDirList(args.get("-rompath"));
            if (fileList.empty()) {
                fprintf(stderr, KRED "ERROR: no files found in rom path\n" KRAS);
                return;
            }
            if (args.exist("-dl")) {
                Io::makedir("media");
                Io::makedir("media/box3d");
                Io::makedir("media/images");
                Io::makedir("media/videos");
            }

            if (args.exist("-threads")) {
                thread_count = (int) std::strtol(args.get("-threads").c_str(), nullptr, 10);
            }
            for (int i = 0; i < thread_count; i++) {
                pthread_create(&threads[i], nullptr, scrap_thread, this);
            }
            for (int i = 0; i < thread_count; i++) {
                pthread_join(threads[i], nullptr);
            }

            gameList.roms_count = gameList.games.size();
            if (!gameList.games.empty() && args.exist("-savexml")) {
                gameList.save(args.get("-savexml"));
            }

            printf(KGRE "\n==========\nALL DONE\n==========\n" KRAS);
            printf(KGRE "found %i games, %zu was not found:" KRAS, gameList.roms_count, missList.size());
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
}

int main(int argc, char **argv) {

    ArgumentParser args(argc, argv);

    // setup screenscraper api
    Api::ss_devid = SS_DEV_ID;
    Api::ss_devpassword = SS_DEV_PWD;
    Api::ss_softname = "sscrap";
    ss_debug = args.exist("-debug");

    auto scrapper = new Scrap(args);
    scrapper->run();

    return 0;
}
