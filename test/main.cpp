//
// Created by cpasjuste on 29/03/19.
//

#include "ss_api.h"
#include "ss_gamelist.h"
#include "args.h"

#define KRED "\x1B[31m"
#define KRAS "\033[0m"

using namespace ss_api;

void printGame(const Game &game) {

    printf("\n===================================\n");
    Game::Name name = game.getName(Game::Country::SS);
    printf("nom (%s): %s (alternatives: %li)\n", name.country.c_str(), name.text.c_str(), game.names.size() - 1);
    for (auto &country : game.countries) {
        printf("country: %s\n", country.c_str());
    }
    printf("id: %s\n", game.id.c_str());
    printf("cloneof: %s\n", game.cloneof.c_str());
    printf("system: %s (id: %s)\n", game.systemename.c_str(), game.systemeid.c_str());
    printf("editor: %s (id: %s)\n", game.editor.text.c_str(), game.editor.id.c_str());
    printf("developer: %s (developpeur: %s)\n", game.developer.text.c_str(), game.developer.id.c_str());
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
    Game::Genre::Name genre = game.getGenre(Game::Language::EN);
    printf("genre (%s): %s\n", genre.language.c_str(), genre.text.c_str());
    // print some medias
    Game::Media media = game.getMedia(Game::Media::Type::SSTitle, Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    //api->download(media, "cache/" + media.type + "_" + media.country + "." + media.format);
    media = game.getMedia(Game::Media::Type::SS, Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    //api->download(media, "cache/" + media.type + "_" + media.country + "." + media.format);
    media = game.getMedia(Game::Media::Type::Mixrbv2, Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    //api->download(media, "cache/" + media.type + "_" + media.country + "." + media.format);
}

int main(int argc, char **argv) {

    // default values
    std::string user;
    std::string pwd;
    Game::Language language = Game::Language::EN;
    ArgumentParser args(argc, argv);

    user = args.get("-u");
    pwd = args.get("-p");
    if (args.exist("-l")) {
        language = Api::toLanguage(args.get("-l"));
        if (language == Game::Language::UNKNOWN) {
            fprintf(stderr, KRED "ERROR: language not found: %s, available languages: en, fr, es, pt\n" KRAS,
                    args.get("-l").c_str());
            return -1;
        }
        printf("language: %s\n", Api::toString(language).c_str());
    }

    // ok, continue..
    Api api(SS_DEV_ID, SS_DEV_PWD, "sscrap");

    if (args.exist("-gameinfo")) {
        //Api::GameInfo gameInfo = api.gameInfo(crc, md5, sha1, "75", "rom", "dino.zip", "", "", SS_ID, SS_PWD);
        Api::GameInfo gameInfo = api.gameInfo(args.get("-crc"), args.get("-md5"), args.get("-sha1"),
                                              args.get("-systemid"), args.get("-romtype"), args.get("-romname"),
                                              args.get("-romsize"), args.get("-gameid"), user, pwd);
        printf("\n===================================\n");
        printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
               gameInfo.ssuser.id.c_str(), gameInfo.ssuser.maxrequestsperday.c_str(),
               gameInfo.ssuser.maxthreads.c_str());
        if (!gameInfo.game.id.empty()) {
            printGame(gameInfo.game);
            // save game list as xml (emulationstation + pFBA compatibility)
            //auto gameList = new GameList();
            //gameList->games.push_back(gameInfo.game);
            //gameList->save("test.xml");
            //delete (gameList);
        } else {
            printf("gameInfo: game not found\n");
        }
    } else if (args.exist("-gamesearch")) {
        Api::GameSearch search = api.gameSearch(args.get("-gamename"), args.get("-systemid"), user, pwd);
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
