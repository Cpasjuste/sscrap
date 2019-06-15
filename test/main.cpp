//
// Created by cpasjuste on 29/03/19.
//

#include "main.h"
#include "ss_api.h"

using namespace ss_api;

void printJeu(Api *api, const Jeu &jeu) {

    printf("\n===================================\n");
    printf("nom: %s (region: %s, alternatives: %li)\n",
           jeu.noms[0].text.c_str(), jeu.noms[0].region.c_str(), jeu.noms.size() - 1);
    for (auto &region : jeu.regions) {
        printf("region: %s\n", region.c_str());
    }
    printf("id: %s\n", jeu.id.c_str());
    printf("editeur: %s (id: %s)\n", jeu.editeur.text.c_str(), jeu.editeur.id.c_str());
    printf("developpeur: %s (developpeur: %s)\n", jeu.developpeur.text.c_str(), jeu.developpeur.id.c_str());
    printf("joueurs: %s\n", jeu.joueurs.c_str());
    printf("note: %s\n", jeu.note.c_str());
    printf("topstaff: %s\n", jeu.topstaff.c_str());
    printf("rotation: %s\n", jeu.rotation.c_str());
    printf("resolution: %s\n", jeu.resolution.c_str());
    printf("controles: %s\n", jeu.controles.c_str());
    printf("couleurs: %s\n", jeu.couleurs.c_str());
    for (auto &synopsis : jeu.synopsis) {
        printf("synopsis (%s): %s\n", synopsis.langue.c_str(), synopsis.text.c_str());
    }
    for (auto &classification : jeu.classifications) {
        printf("classification: %s: %s\n", classification.type.c_str(), classification.text.c_str());
    }
    for (auto &date : jeu.dates) {
        printf("date (%s): %s\n", date.region.c_str(), date.text.c_str());
    }
    for (auto &genre : jeu.genres) {
        printf("genre: %s (%s)\n", genre.noms.empty() ? "N/A" : genre.noms[0].text.c_str(), genre.id.c_str());
    }
    for (auto &famille : jeu.familles) {
        printf("famille: %s (%s)\n", famille.noms.empty() ? "N/A" : famille.noms[0].text.c_str(), famille.id.c_str());
    }
    std::vector<Jeu::Media> medias = api->getMedia(jeu, Jeu::Media::Type::Mixrbv2, Api::Region::WOR);
    for (auto &media : medias) {
        printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
        //api->download(media, "cache/" + media.type + "_" + media.region + "." + media.format);
    }
}

int main() {

    Api api(SS_DEV_ID, SS_DEV_PWD, "SSSCRAP");

    Api::JeuRecherche recherche = api.jeuRecherche("sonic", "1", SS_ID, SS_PWD);
    printf("\n===================================\n");
    printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
           recherche.ssuser.id.c_str(), recherche.ssuser.maxrequestsperday.c_str(),
           recherche.ssuser.maxthreads.c_str());
    printf("games found: %li\n", recherche.jeux.size());
    for (auto &jeu : recherche.jeux) {
        printJeu(&api, jeu);
    }

    Api::JeuInfos jeuInfos = api.jeuInfos("", "", "", "75", "rom", "dino.zip", "", "", SS_ID, SS_PWD);
    printf("\n===================================\n");
    printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
           jeuInfos.ssuser.id.c_str(), jeuInfos.ssuser.maxrequestsperday.c_str(),
           jeuInfos.ssuser.maxthreads.c_str());
    if (!jeuInfos.jeu.id.empty()) {
        printJeu(&api, jeuInfos.jeu);
    } else {
        printf("jeuInfos: game not found\n");
    }

    return 0;
}
