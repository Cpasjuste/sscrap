//
// Created by cpasjuste on 29/03/19.
//

#include "ss_api.h"

using namespace ss_api;

void printGame(const Game &game) {

    printf("\n===================================\n");
    printf("nom: %s (region: %s, alternatives: %li)\n",
           game.names[0].text.c_str(), game.names[0].country.c_str(), game.names.size() - 1);
    for (auto &region : game.countries) {
        printf("region: %s\n", region.c_str());
    }
    printf("id: %s\n", game.id.c_str());
    printf("editeur: %s (id: %s)\n", game.editor.text.c_str(), game.editor.id.c_str());
    printf("developpeur: %s (developpeur: %s)\n", game.developer.text.c_str(), game.developer.id.c_str());
    printf("joueurs: %s\n", game.players.c_str());
    printf("note: %s\n", game.rating.c_str());
    printf("topstaff: %s\n", game.topstaff.c_str());
    printf("rotation: %s\n", game.rotation.c_str());
    printf("resolution: %s\n", game.resolution.c_str());
    printf("controles: %s\n", game.inputs.c_str());
    printf("couleurs: %s\n", game.colors.c_str());
    for (auto &synopsis : game.synopsis) {
        printf("synopsis (%s): %s\n", synopsis.language.c_str(), synopsis.text.c_str());
    }
    for (auto &classification : game.classifications) {
        printf("classification: %s: %s\n", classification.type.c_str(), classification.text.c_str());
    }
    for (auto &date : game.dates) {
        printf("date (%s): %s\n", date.country.c_str(), date.text.c_str());
    }
    for (auto &genre : game.genres) {
        printf("genre: %s (%s)\n", genre.names.empty() ? "N/A" : genre.names[0].text.c_str(), genre.id.c_str());
    }
    for (auto &famille : game.families) {
        printf("famille: %s (%s)\n", famille.names.empty() ? "N/A" : famille.names[0].text.c_str(), famille.id.c_str());
    }
    std::vector<Game::Media> medias = Api::getMedia(game, Game::Media::Type::Mixrbv2, Api::Country::WOR);
    for (auto &media : medias) {
        printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
        //api->download(media, "cache/" + media.type + "_" + media.country + "." + media.format);
    }
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
    } else {
        printf("jeuInfos: game not found\n");
    }

    return 0;
}
