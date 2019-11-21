//
// Created by cpasjuste on 29/03/19.
//

#include "ss_api.h"
#include "ss_io.h"
#include "args.h"

#define KRED "\x1B[31m"
#define KGRE "\x1B[92m"
#define KRAS "\033[0m"

using namespace ss_api;

std::vector<std::string> missList;

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

int main(int argc, char **argv) {

    // default values
    std::string user;
    std::string pwd;
    Game::Language language = Game::Language::EN;
    ArgumentParser args(argc, argv);

    user = args.get("-user");
    pwd = args.get("-password");
    if (args.exist("-language")) {
        language = Api::toLanguage(args.get("-language"));
        if (language == Game::Language::UNKNOWN) {
            fprintf(stderr, KRED "ERROR: language not found: %s, available languages: en, fr, es, pt\n" KRAS,
                    args.get("-language").c_str());
            return -1;
        }
        printf("language: %s\n", Api::toString(language).c_str());
    }

    // setup screenscraper api
    Api::ss_devid = SS_DEV_ID;
    Api::ss_devpassword = SS_DEV_PWD;
    Api::ss_softname = "sscrap";
    ss_debug = args.exist("-debug");

    if (args.exist("-gameinfo")) {
        if (args.exist("-rompath")) {
            std::vector<std::string> files = Io::getDirList(args.get("-rompath"));
            if (files.empty()) {
                fprintf(stderr, KRED "ERROR: no files found in rom path\n" KRAS);
                return -1;
            }
            Io::makedir("media");
            Io::makedir("media/box3d");
            Io::makedir("media/images");
            Io::makedir("media/videos");
            Api::GameList gameList;
            for (const auto &file : files) {
                Api::GameInfo gameInfo = Api::gameInfo("", "", "",
                                                       args.get("-systemid"), args.get("-romtype"),
                                                       file, "", "", user, pwd);
                if (!gameInfo.game.id.empty()) {
                    printf(KGRE "OK: %s => %s (%s)\n" KRAS,
                           file.c_str(), gameInfo.game.getName().text.c_str(), gameInfo.game.system.text.c_str());
                    //gameInfo.game.
                    gameList.games.emplace_back(gameInfo.game);
                } else {
                    fprintf(stderr, KRED "NOK: %s\n" KRAS, file.c_str());
                    missList.emplace_back(file);
                };
            }
            gameList.roms_count = gameList.games.size();
            if (!gameList.games.empty() && args.exist("-savexml")) {
                gameList.save(args.get("-savexml"));
            }
            printf(KGRE "\n==========\nALL DONE\n==========\n" KRAS);
            printf(KGRE "found %i games on %zu files" KRAS, gameList.roms_count, files.size());
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

    return 0;
}
