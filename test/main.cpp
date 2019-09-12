//
// Created by cpasjuste on 29/03/19.
//

#include "ss_api.h"
#include "ss_gamelist.h"

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

int main() {

    Api api(SS_DEV_ID, SS_DEV_PWD, "SSSCRAP");

    Api::GameSearch search = api.gameSearch("sonic", "1", SS_ID, SS_PWD);
    printf("\n===================================\n");
    printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
           search.ssuser.id.c_str(), search.ssuser.maxrequestsperday.c_str(),
           search.ssuser.maxthreads.c_str());
    printf("games found: %li\n", search.games.size());
    for (auto &game : search.games) {
        printGame(game);
    }

    Api::GameInfo gameInfo = api.gameInfo("", "", "", "75", "rom", "dino.zip", "", "", SS_ID, SS_PWD);
    printf("\n===================================\n");
    printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
           gameInfo.ssuser.id.c_str(), gameInfo.ssuser.maxrequestsperday.c_str(),
           gameInfo.ssuser.maxthreads.c_str());
    if (!gameInfo.game.id.empty()) {
        printGame(gameInfo.game);
        // save game list as xml (emulationstation + pFBA compatibility)
        auto gameList = new GameList();
        gameList->games.push_back(gameInfo.game);
        gameList->save("test.xml");
        delete (gameList);
    } else {
        printf("jeuInfos: game not found\n");
    }

    return 0;
}
