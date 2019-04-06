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
                      + systemeid + "&recherche=" + search;

    if (!ssid.empty()) {
        url += "&ssid=" + ssid;
    }
    if (!sspassword.empty()) {
        url += "&sspassword=" + sspassword;
    }

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
    } else {
        jr.ssuser = parseUser(json_ssuser);
    }

    // search and parse "jeux" object (array)
    found = json_object_object_get_ex(json_response, "jeux", &json_jeux);
    if (!found) {
        printf("Api::parseJeuRecherche: error: jeux object not found\n");
    }

    int count = json_object_array_length(json_jeux);
    for (int i = 0; i < count; i++) {
        json_object *json_jeu = json_object_array_get_idx(json_jeux, i);
        if (getJsonString(json_jeu, "id").empty()) {
            continue;
        }
        jr.jeux.push_back(parseJeu(json_jeu));
    }

    return jr;
}

Api::JeuInfos
Api::jeuInfos(const std::string &crc, const std::string &md5, const std::string &sha1, const std::string &systemeid,
              const std::string &romtype, const std::string &romnom, const std::string &romtaille,
              const std::string &gameid, const std::string &ssid, const std::string &sspassword) {

    long code = 0;
    std::string search = curl.escape(romnom);
    std::string soft = curl.escape(softname);

    std::string url = "https://www.screenscraper.fr/api2/jeuInfos.php?devid="
                      + devid + "&devpassword=" + devpassword + "&softname=" + soft + "&output=json"
                      + "&systemeid=" + systemeid + "&romnom=" + search;

    if (!ssid.empty()) {
        url += "&ssid=" + ssid;
    }
    if (!sspassword.empty()) {
        url += "&sspassword=" + sspassword;
    }
    if (!crc.empty()) {
        url += "&crc=" + crc;
    }
    if (!md5.empty()) {
        url += "&md5=" + md5;
    }
    if (!sha1.empty()) {
        url += "&sha1=" + sha1;
    }
    if (!systemeid.empty()) {
        url += "&systemeid=" + systemeid;
    }
    if (!romtype.empty()) {
        url += "&romtype=" + romtype;
    }
    if (!romtaille.empty()) {
        url += "&romtaille=" + romtaille;
    }
    if (!gameid.empty()) {
        url += "&gameid=" + gameid;
    }

    printf("Api::jeuInfos: %s\n", url.c_str());

    std::string json = curl.getString(url, 10, &code);
    if (json.empty()) {
        printf("Api::jeuInfos: error %li\n", code);
        return JeuInfos();
    }

    return parseJeuInfos(json);
}

Api::JeuInfos Api::parseJeuInfos(const std::string &jsonData) {

    JeuInfos ji{};
    json_object *json_root = json_tokener_parse(jsonData.c_str());
    json_object *json_response, *json_ssuser, *json_jeu;

    json_bool found = json_object_object_get_ex(json_root, "response", &json_response);
    if (!found) {
        printf("Api::parseJeuInfos: error: response object not found\n");
        return ji;
    }

    // search and parse "ssuser" object
    found = json_object_object_get_ex(json_response, "ssuser", &json_ssuser);
    if (!found) {
        printf("Api::parseJeuInfos: error: ssuser object not found\n");
    } else {
        ji.ssuser = parseUser(json_ssuser);
    }

    // search and parse "jeu" object
    found = json_object_object_get_ex(json_response, "jeu", &json_jeu);
    if (!found) {
        printf("Api::parseJeuInfos: error: jeu object not found\n");
    }

    if (!getJsonString(json_jeu, "id").empty()) {
        ji.jeu = parseJeu(json_jeu);
    }

    return ji;
}

Jeu Api::parseJeu(json_object *root) {

    Jeu jeu;

    jeu.id = getJsonString(root, "id");
    jeu.romid = getJsonString(root, "romid");
    jeu.notgame = getJsonString(root, "notgame");
    // parse names array
    json_object *name_array = getJsonObject(root, "noms");
    if (name_array) {
        int name_size = json_object_array_length(name_array);
        for (int j = 0; j < name_size; j++) {
            json_object *json_nom = json_object_array_get_idx(name_array, j);
            jeu.noms.push_back({getJsonString(json_nom, "region"), getJsonString(json_nom, "text")});
        }
    }
    jeu.cloneof = getJsonString(root, "cloneof");
    jeu.systemeid = getJsonString(root, "systemeid");
    jeu.systemenom = getJsonString(root, "systemenom");
    // parse editor object
    jeu.editeur.id = getJsonString(getJsonObject(root, "editeur"), "id");
    jeu.editeur.text = getJsonString(getJsonObject(root, "editeur"), "text");
    jeu.developpeur.id = getJsonString(getJsonObject(root, "developpeur"), "id");
    jeu.developpeur.text = getJsonString(getJsonObject(root, "developpeur"), "text");
    jeu.joueurs = getJsonString(getJsonObject(root, "joueurs"), "text");
    jeu.note = getJsonString(getJsonObject(root, "note"), "text");
    jeu.topstaff = getJsonString(root, "topstaff");
    jeu.rotation = getJsonString(root, "rotation");
    jeu.resolution = getJsonString(root, "resolution");
    jeu.controles = getJsonString(root, "controles");
    jeu.couleurs = getJsonString(root, "couleurs");
    // parse synopsis array
    json_object *synopsis_array = getJsonObject(root, "synopsis");
    if (synopsis_array) {
        int synopsis_size = json_object_array_length(synopsis_array);
        for (int j = 0; j < synopsis_size; j++) {
            json_object *json_syn = json_object_array_get_idx(synopsis_array, j);
            jeu.synopsis.push_back({getJsonString(json_syn, "langue"), getJsonString(json_syn, "text")});
        }
    }

    return jeu;
}

User Api::parseUser(json_object *root) {

    User user;

    user.id = getJsonString(root, "id");
    user.niveau = getJsonString(root, "niveau");
    user.contribution = getJsonString(root, "contribution");
    user.uploadsysteme = getJsonString(root, "uploadsysteme");
    user.uploadinfos = getJsonString(root, "uploadinfos");
    user.romasso = getJsonString(root, "romasso");
    user.uploadmedia = getJsonString(root, "uploadmedia");
    user.maxthreads = getJsonString(root, "maxthreads");
    user.maxdownloadspeed = getJsonString(root, "maxdownloadspeed");
    user.requeststoday = getJsonString(root, "requeststoday");
    user.maxrequestsperday = getJsonString(root, "maxrequestsperday");
    user.visites = getJsonString(root, "visites");
    user.datedernierevisite = getJsonString(root, "datedernierevisite");
    user.favregion = getJsonString(root, "favregion");

    return user;
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

