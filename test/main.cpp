//
// Created by cpasjuste on 29/03/19.
//

#include "main.h"
#include "ss_api.h"

using namespace ss_api;

void printJeu(const Jeu &jeu) {

    printf("\n===================================\n");
    printf("nom: %s (region: %s, alternatives: %li)\n",
           jeu.noms[0].text.c_str(), jeu.noms[0].region.c_str(), jeu.noms.size() - 1);
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
    if (!jeu.synopsis.empty()) {
        printf("synopsis (langue: %s, alternatives: %li): %s\n",
               jeu.synopsis[0].langue.c_str(), jeu.synopsis.size() - 1, jeu.synopsis[0].text.c_str());
    }
}

int main() {

    Api api(DEVID, DEVPWD, "SSSCRAP");

    Api::JeuRecherche recherche = api.jeuRecherche("cadillacs", "75", SSID, SSPWD);
    printf("\n===================================\n");
    printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
           recherche.ssuser.id.c_str(), recherche.ssuser.maxrequestsperday.c_str(),
           recherche.ssuser.maxthreads.c_str());
    printf("games found: %li\n", recherche.jeux.size());
    for (auto &jeu : recherche.jeux) {
        printJeu(jeu);
    }

    Api::JeuInfos jeuInfos = api.jeuInfos("", "", "", "75", "rom", "dino.zip", "", "", SSID, SSPWD);
    printf("\n===================================\n");
    printf("ss_username: %s (maxrequestsperday: %s, maxthreads: %s)\n",
           jeuInfos.ssuser.id.c_str(), jeuInfos.ssuser.maxrequestsperday.c_str(),
           jeuInfos.ssuser.maxthreads.c_str());
    if (!jeuInfos.jeu.id.empty()) {
        printJeu(jeuInfos.jeu);
    } else {
        printf("jeuInfos: game not found\n");
    }

    return 0;
}
