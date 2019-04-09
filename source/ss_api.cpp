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
                      + devid + "&devpassword=" + devpassword + "&softname=" + soft + "&output=json"
                      + "&recherche=" + search;

    if (!ssid.empty()) {
        url += "&ssid=" + ssid;
    }
    if (!sspassword.empty()) {
        url += "&sspassword=" + sspassword;
    }
    if (!systemeid.empty()) {
        url += "&systemeid=" + systemeid;
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
                      + "&romnom=" + search;

    if (!ssid.empty()) {
        url += "&ssid=" + ssid;
    }
    if (!sspassword.empty()) {
        url += "&sspassword=" + sspassword;
    }
    if (!systemeid.empty()) {
        url += "&systemeid=" + systemeid;
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
    json_object *array;

    jeu.id = getJsonString(root, "id");
    jeu.romid = getJsonString(root, "romid");
    jeu.notgame = getJsonString(root, "notgame");
    // parse names array
    array = getJsonObject(root, "noms");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            jeu.noms.push_back({getJsonString(json_obj, "region"), getJsonString(json_obj, "text")});
        }
    }
    // parse region array
    std::string region = getJsonString(getJsonObject(root, "regions"), "shortname");
    if (!region.empty()) {
        jeu.regions.push_back(region);
    } else {
        array = getJsonObject(root, "regions");
        if (array) {
            int size = json_object_array_length(array);
            for (int j = 0; j < size; j++) {
                json_object *json_obj = json_object_array_get_idx(array, j);
                jeu.regions.push_back(getJsonString(json_obj, "shortname"));
            }
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
    array = getJsonObject(root, "synopsis");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_syn = json_object_array_get_idx(array, j);
            jeu.synopsis.push_back({getJsonString(json_syn, "langue"), getJsonString(json_syn, "text")});
        }
    }
    // parse classification array
    array = getJsonObject(root, "classifications");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            jeu.classifications.push_back({getJsonString(json_obj, "type"), getJsonString(json_obj, "text")});
        }
    }
    // parse dates array
    array = getJsonObject(root, "dates");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            jeu.dates.push_back({getJsonString(json_obj, "region"), getJsonString(json_obj, "text")});
        }
    }
    // parse genres array
    array = getJsonObject(root, "genres");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            Jeu::Genre genre;
            genre.id = getJsonString(json_obj, "id");
            genre.principale = getJsonString(json_obj, "principale");
            genre.parentid = getJsonString(json_obj, "parentid");
            json_object *sub_array = getJsonObject(json_obj, "noms");
            if (!sub_array) {
                sub_array = getJsonObject(json_obj, "names");
            }
            if (sub_array) {
                int sub_size = json_object_array_length(sub_array);
                for (int k = 0; k < sub_size; k++) {
                    json_object *json_sub_obj = json_object_array_get_idx(sub_array, k);
                    genre.noms.push_back({getJsonString(json_sub_obj, "langue"),
                                          getJsonString(json_sub_obj, "text")});
                }
            }
            jeu.genres.push_back(genre);
        }
    }
    // parse familles array
    array = getJsonObject(root, "familles");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            Jeu::Famille famille;
            famille.id = getJsonString(json_obj, "id");
            famille.principale = getJsonString(json_obj, "principale");
            famille.parentid = getJsonString(json_obj, "parentid");
            json_object *sub_array = getJsonObject(json_obj, "noms");
            if (!sub_array) {
                sub_array = getJsonObject(json_obj, "names");
            }
            if (sub_array) {
                int sub_size = json_object_array_length(sub_array);
                for (int k = 0; k < sub_size; k++) {
                    json_object *json_sub_obj = json_object_array_get_idx(sub_array, k);
                    famille.noms.push_back({getJsonString(json_sub_obj, "langue"),
                                            getJsonString(json_sub_obj, "text")});
                }
            }
            jeu.familles.push_back(famille);
        }
    }
    // parse medias array
    array = getJsonObject(root, "medias");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            Jeu::Media media;
            json_object *json_obj = json_object_array_get_idx(array, j);
            media.type = getJsonString(json_obj, "type");
            media.parent = getJsonString(json_obj, "parent");
            media.url = getJsonString(json_obj, "url");
            media.region = getJsonString(json_obj, "region");
            media.crc = getJsonString(json_obj, "crc");
            media.md5 = getJsonString(json_obj, "md5");
            media.sha1 = getJsonString(json_obj, "sha1");
            media.format = getJsonString(json_obj, "format");
            jeu.medias.push_back(media);
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

