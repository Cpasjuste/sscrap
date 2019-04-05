//
// Created by cpasjuste on 05/04/19.
//

#include <cstring>

#include "ss_api.h"

using namespace ss_api;

Api::Api(const std::string &_devid, const std::string &_devpassword,
         const std::string &_softname) {

    devid = _devid;
    devpassword = _devpassword;
    softname = _softname;
}

Api::JeuRecherche Api::jeuRecherche(const std::string &recherche, const std::string &systemeid,
                                    const std::string &ssid, const std::string &sspassword) {

    long code = 0;
    std::string search = curl.escape(recherche);
    std::string soft = curl.escape(softname);

    std::string url = "https://www.screenscraper.fr/api2/jeuRecherche.php?devid="
                      + devid + "&devpassword=" + devpassword + "&softname=" + soft + "&output=json&ssid="
                      + ssid + "&sspassword=" + sspassword + "&systemeid=" + systemeid + "&recherche=" + search;

    printf("Api::jeuRecherche: %s\n", url.c_str());

    std::string json = curl.getString(url, 10, &code);
    if (json.empty()) {
        printf("Api::jeuRecherche: error %li\n", code);
        return JeuRecherche();
    }

    return parseJeuRecherche(json);
}

Api::JeuRecherche Api::parseJeuRecherche(const std::string &jsonData) {

    JeuRecherche jr{};
    json_object *json_root = json_tokener_parse(jsonData.c_str());
    json_object *json_response, *json_ssuser, *json_jeux;

    json_bool found = json_object_object_get_ex(json_root, "response", &json_response);
    if (!found) {
        printf("Api::parseJeuRecherche: error: response object not found\n");
        return jr;
    }

    // search and parse "ssuser" object
    found = json_object_object_get_ex(json_response, "ssuser", &json_ssuser);
    if (!found) {
        printf("Api::parseJeuRecherche: error: ssuser object not found\n");
        return jr;
    }

    jr.ssuser.id = getJsonString(json_ssuser, "id");
    jr.ssuser.niveau = getJsonString(json_ssuser, "niveau");
    jr.ssuser.contribution = getJsonString(json_ssuser, "contribution");
    jr.ssuser.uploadsysteme = getJsonString(json_ssuser, "uploadsysteme");
    jr.ssuser.uploadinfos = getJsonString(json_ssuser, "uploadinfos");
    jr.ssuser.romasso = getJsonString(json_ssuser, "romasso");
    jr.ssuser.uploadmedia = getJsonString(json_ssuser, "uploadmedia");
    jr.ssuser.maxthreads = getJsonString(json_ssuser, "maxthreads");
    jr.ssuser.maxdownloadspeed = getJsonString(json_ssuser, "maxdownloadspeed");
    jr.ssuser.requeststoday = getJsonString(json_ssuser, "requeststoday");
    jr.ssuser.maxrequestsperday = getJsonString(json_ssuser, "maxrequestsperday");
    jr.ssuser.visites = getJsonString(json_ssuser, "visites");
    jr.ssuser.datedernierevisite = getJsonString(json_ssuser, "datedernierevisite");
    jr.ssuser.favregion = getJsonString(json_ssuser, "favregion");

    // search and parse "jeux" object (array)
    found = json_object_object_get_ex(json_response, "jeux", &json_jeux);
    if (!found) {
        printf("Api::parseJeuRecherche: error: jeux object not found\n");
        return jr;
    }

    int count = json_object_array_length(json_jeux);
    for (int i = 0; i < count; i++) {
        json_object *json_jeu = json_object_array_get_idx(json_jeux, i);
        if (getJsonString(json_jeu, "id").empty()) {
            continue;
        }
        Jeu jeu;
        jeu.id = getJsonString(json_jeu, "id");
        jeu.romid = getJsonString(json_jeu, "romid");
        jeu.notgame = getJsonString(json_jeu, "notgame");
        // parse names array
        json_object *name_array = getJsonObject(json_jeu, "noms");
        if (name_array) {
            int name_size = json_object_array_length(name_array);
            for (int j = 0; j < name_size; j++) {
                json_object *json_nom = json_object_array_get_idx(name_array, j);
                jeu.noms.push_back({getJsonString(json_nom, "region"), getJsonString(json_nom, "text")});
            }
        }
        jeu.cloneof = getJsonString(json_jeu, "cloneof");
        jeu.systemeid = getJsonString(json_jeu, "systemeid");
        jeu.systemenom = getJsonString(json_jeu, "systemenom");
        // parse editor object
        jeu.editeur.id = getJsonString(getJsonObject(json_jeu, "editeur"), "id");
        jeu.editeur.text = getJsonString(getJsonObject(json_jeu, "editeur"), "text");
        jeu.developpeur.id = getJsonString(getJsonObject(json_jeu, "developpeur"), "id");
        jeu.developpeur.text = getJsonString(getJsonObject(json_jeu, "developpeur"), "text");
        jeu.joueurs = getJsonString(getJsonObject(json_jeu, "joueurs"), "text");
        jeu.note = getJsonString(getJsonObject(json_jeu, "note"), "text");
        jeu.topstaff = getJsonString(json_jeu, "topstaff");
        jeu.rotation = getJsonString(json_jeu, "rotation");
        jeu.resolution = getJsonString(json_jeu, "resolution");
        jeu.controles = getJsonString(json_jeu, "controles");
        jeu.couleurs = getJsonString(json_jeu, "couleurs");
        // parse synopsis array
        json_object *synopsis_array = getJsonObject(json_jeu, "synopsis");
        if (synopsis_array) {
            int synopsis_size = json_object_array_length(synopsis_array);
            for (int j = 0; j < synopsis_size; j++) {
                json_object *json_syn = json_object_array_get_idx(synopsis_array, j);
                jeu.synopsis.push_back({getJsonString(json_syn, "langue"), getJsonString(json_syn, "text")});
            }
        }

        jr.jeux.push_back(jeu);

        //printf("%s: %s, %s\n", jeu.id.c_str(), jeu.systemenom.c_str(), jeu.joueurs.c_str());
        //json_object_object_foreach(json_jeu, key, obj) {
        //    printf("%s: %s\n", json_type_to_name(json_object_get_type(obj)),
        //           json_object_get_string(obj));
        //}
    }

    return jr;
}

json_object *Api::getJsonObject(json_object *root, const std::string &key) {

    json_object *object;

    if (root) {
        json_bool found = json_object_object_get_ex(root, key.c_str(), &object);
        if (found) {
            return object;
        }
    }

    return nullptr;
}

std::string Api::getJsonString(json_object *root, const std::string &key) {

    json_object *object;

    if (root) {
        json_bool found = json_object_object_get_ex(root, key.c_str(), &object);
        if (found && object) {
            return json_object_get_string(object);
        }
    }

    return "";
}
