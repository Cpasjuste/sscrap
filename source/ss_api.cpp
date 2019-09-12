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

Api::GameSearch Api::gameSearch(const std::string &recherche, const std::string &systemeid,
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

    std::string json = curl.getString(url, SS_TIMEOUT, &code);
    if (json.empty()) {
        printf("Api::jeuRecherche: error %li\n", code);
        return GameSearch();
    }

    return parseGameSearch(json);
}

Api::GameSearch Api::gameSearch(const std::string &srcPath) {

    long size = 0;
    FILE *fp = fopen(srcPath.c_str(), "rb");
    if (!fp) {
        printf("Api::jeuRecherche: error: fopen failed\n");
        return GameSearch();
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size <= 0) {
        printf("Api::jeuRecherche: error: ftell failed\n");
        fclose(fp);
        return GameSearch();
    }
    fseek(fp, 0, SEEK_SET);
    std::string json = std::string((unsigned long) size, '\0');
    fread(&json[0], sizeof(char), (size_t) size, fp);
    fclose(fp);

    if (json.empty()) {
        printf("Api::jeuRecherche: error: data is empty\n");
        return GameSearch();
    }

    return parseGameSearch(json);
}

Api::GameSearch Api::parseGameSearch(const std::string &jsonData) {

    GameSearch jr{};
    jr.json = jsonData;
    json_object *json_root = json_tokener_parse(jr.json.c_str());
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
        jr.games.push_back(parseGame(json_jeu));
    }

    return jr;
}

Api::GameInfo
Api::gameInfo(const std::string &crc, const std::string &md5, const std::string &sha1, const std::string &systemeid,
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

    std::string json = curl.getString(url, SS_TIMEOUT, &code);
    if (json.empty()) {
        printf("Api::jeuInfos: error %li\n", code);
        return GameInfo();
    }

    return parseGameInfo(json);
}

Api::GameInfo Api::gameInfo(const std::string &srcPath) {

    long size = 0;
    FILE *fp = fopen(srcPath.c_str(), "rb");
    if (!fp) {
        printf("Api::jeuInfos: error: fopen failed: %s\n", srcPath.c_str());
        return GameInfo();
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size <= 0) {
        printf("Api::jeuInfos: error: ftell failed\n");
        fclose(fp);
        return GameInfo();
    }
    fseek(fp, 0, SEEK_SET);
    std::string json = std::string((unsigned long) size, '\0');
    fread(&json[0], sizeof(char), (size_t) size, fp);
    fclose(fp);

    if (json.empty()) {
        printf("Api::jeuInfos: error: data is empty\n");
        return GameInfo();
    }

    return parseGameInfo(json);
}

Api::GameInfo Api::parseGameInfo(const std::string &jsonData) {

    GameInfo ji{};
    ji.json = jsonData;
    json_object *json_root = json_tokener_parse(ji.json.c_str());
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
        ji.game = parseGame(json_jeu);
    }

    return ji;
}

Game Api::parseGame(json_object *root) {

    Game game;
    json_object *array;

    game.source = "screenscraper.fr";
    game.id = getJsonString(root, "id");
    game.romid = getJsonString(root, "romid");
    game.notgame = getJsonString(root, "notgame");
    // parse names array
    array = getJsonObject(root, "noms");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            game.names.push_back({getJsonString(json_obj, "region"), getJsonString(json_obj, "text")});
        }
    }
    // parse region array
    std::string region = getJsonString(getJsonObject(root, "regions"), "shortname");
    if (!region.empty()) {
        game.countries.push_back(region);
    } else {
        array = getJsonObject(root, "regions");
        if (array) {
            int size = json_object_array_length(array);
            for (int j = 0; j < size; j++) {
                json_object *json_obj = json_object_array_get_idx(array, j);
                game.countries.push_back(getJsonString(json_obj, "shortname"));
            }
        }
    }
    game.cloneof = getJsonString(root, "cloneof");
    game.systemeid = getJsonString(root, "systemeid");
    game.systemename = getJsonString(root, "systemenom");
    // parse editor object
    game.editor.id = getJsonString(getJsonObject(root, "editeur"), "id");
    game.editor.text = getJsonString(getJsonObject(root, "editeur"), "text");
    game.developer.id = getJsonString(getJsonObject(root, "developpeur"), "id");
    game.developer.text = getJsonString(getJsonObject(root, "developpeur"), "text");
    game.players = getJsonString(getJsonObject(root, "joueurs"), "text");
    game.rating = getJsonString(getJsonObject(root, "note"), "text");
    game.topstaff = getJsonString(root, "topstaff");
    game.rotation = getJsonString(root, "rotation");
    game.resolution = getJsonString(root, "resolution");
    game.inputs = getJsonString(root, "controles");
    game.colors = getJsonString(root, "couleurs");
    // parse synopsis array
    array = getJsonObject(root, "synopsis");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_syn = json_object_array_get_idx(array, j);
            game.synopses.push_back({getJsonString(json_syn, "langue"), getJsonString(json_syn, "text")});
        }
    }
    // parse classification array
    array = getJsonObject(root, "classifications");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            game.classifications.push_back({getJsonString(json_obj, "type"), getJsonString(json_obj, "text")});
        }
    }
    // parse dates array
    array = getJsonObject(root, "dates");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            game.dates.push_back({getJsonString(json_obj, "region"), getJsonString(json_obj, "text")});
        }
    }
    // parse genres array
    array = getJsonObject(root, "genres");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            Game::Genre genre;
            genre.id = getJsonString(json_obj, "id");
            genre.main = getJsonString(json_obj, "principale");
            genre.parentid = getJsonString(json_obj, "parentid");
            json_object *sub_array = getJsonObject(json_obj, "noms");
            if (sub_array) {
                int sub_size = json_object_array_length(sub_array);
                for (int k = 0; k < sub_size; k++) {
                    json_object *json_sub_obj = json_object_array_get_idx(sub_array, k);
                    genre.names.push_back({getJsonString(json_sub_obj, "langue"),
                                           getJsonString(json_sub_obj, "text")});
                }
            }
            game.genres.push_back(genre);
        }
    }
    // parse familles array
    array = getJsonObject(root, "familles");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            json_object *json_obj = json_object_array_get_idx(array, j);
            Game::Family famille;
            famille.id = getJsonString(json_obj, "id");
            famille.main = getJsonString(json_obj, "principale");
            famille.parentid = getJsonString(json_obj, "parentid");
            json_object *sub_array = getJsonObject(json_obj, "noms");
            if (sub_array) {
                int sub_size = json_object_array_length(sub_array);
                for (int k = 0; k < sub_size; k++) {
                    json_object *json_sub_obj = json_object_array_get_idx(sub_array, k);
                    famille.names.push_back({getJsonString(json_sub_obj, "langue"),
                                             getJsonString(json_sub_obj, "text")});
                }
            }
            game.families.push_back(famille);
        }
    }
    // parse medias array
    array = getJsonObject(root, "medias");
    if (array) {
        int size = json_object_array_length(array);
        for (int j = 0; j < size; j++) {
            Game::Media media;
            json_object *json_obj = json_object_array_get_idx(array, j);
            media.type = getJsonString(json_obj, "type");
            media.parent = getJsonString(json_obj, "parent");
            media.url = getJsonString(json_obj, "url");
            media.country = getJsonString(json_obj, "region");
            media.crc = getJsonString(json_obj, "crc");
            media.md5 = getJsonString(json_obj, "md5");
            media.sha1 = getJsonString(json_obj, "sha1");
            media.format = getJsonString(json_obj, "format");
            media.support = getJsonString(json_obj, "support");
            game.medias.push_back(media);
        }
    }

    return game;
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

/*
std::vector<Game::Media> Api::getMedia(const Game &game, const Game::Media::Type::Type &type, const Country &country) {

    std::vector<Game::Media> medias;

    remove_copy_if(game.medias.begin(), game.medias.end(), back_inserter(medias),
                   [type, country](const Game::Media &media) {
                       return media.type != mediaTypeToString(type)
                              || (country != Country::ALL && media.country != countryToString(country));
                   });

    return medias;
}
*/

int Api::download(const Game::Media &media, const std::string &dstPath) {

    if (dstPath.empty()) {
        return -1;
    }

    printf("Api::download: %s\n", media.url.c_str());

    long http_code = 0;
    int res = curl.getData(media.url, dstPath, SS_TIMEOUT, &http_code);
    if (res != 0) {
        printf("Api::download: error: curl failed: %s, http_code: %li\n",
               curl_easy_strerror((CURLcode) res), http_code);
        return (int) http_code;
    }

    return 0;
}

bool Api::GameInfo::save(const std::string &dstPath) {

    FILE *fp = fopen(dstPath.c_str(), "wb");
    if (!fp) {
        printf("Api::JeuInfos::save: error: fopen failed\n");
        return -1;
    }

    fwrite(json.c_str(), sizeof(char), json.length(), fp);
    fclose(fp);

    return true;
}

bool Api::GameSearch::save(const std::string &dstPath) {
    FILE *fp = fopen(dstPath.c_str(), "wb");
    if (!fp) {
        printf("Api::JeuRecherche::save: error: fopen failed\n");
        return -1;
    }

    fwrite(json.c_str(), sizeof(char), json.length(), fp);
    fclose(fp);

    return true;
}

std::string Api::toString(const Game::Media::Type &type) {
    switch (type) {
        case Game::Media::Type::SSTitle:
            return "sstitle";
        case Game::Media::Type::SS:
            return "ss";
        case Game::Media::Type::Screenshot:
            return "screenshot";
        case Game::Media::Type::Fanart:
            return "fanart";
        case Game::Media::Type::Video:
            return "video";
        case Game::Media::Type::Marquee:
            return "marquee";
        case Game::Media::Type::ScreenMarquee:
            return "screenmarquee";
        case Game::Media::Type::ScreenMarqueeSmall:
            return "screenmarqueesmall";
        case Game::Media::Type::ThemeHs:
            return "themehs";
        case Game::Media::Type::Manuel:
            return "manuel";
        case Game::Media::Type::Flyer:
            return "flyer";
        case Game::Media::Type::SteamGrid:
            return "steamgrid";
        case Game::Media::Type::Wheel:
            return "wheel";
        case Game::Media::Type::WheelHD:
            return "wheel-hd";
        case Game::Media::Type::WheelCarbon:
            return "wheel-carbon";
        case Game::Media::Type::WheelSteel:
            return "wheel-steel";
        case Game::Media::Type::Box2D:
            return "box-2D";
        case Game::Media::Type::Box2DSide:
            return "box-2D-side";
        case Game::Media::Type::Box2DBack:
            return "box-2D-back";
        case Game::Media::Type::BoxTexture:
            return "box-texture";
        case Game::Media::Type::Box3D:
            return "box-3D";
        case Game::Media::Type::BoxScan:
            return "box-scan";
        case Game::Media::Type::SupportTexture:
            return "support-texture";
        case Game::Media::Type::Bezel43:
            return "bezel-4-3";
        case Game::Media::Type::Bezel169:
            return "bezel-16-9";
        case Game::Media::Type::Bezel1610:
            return "bezel-16-10";
        case Game::Media::Type::Mixrbv1:
            return "mixrbv1";
        case Game::Media::Type::Mixrbv2:
            return "mixrbv2";
        case Game::Media::Type::Pictoliste:
            return "pictoliste";
        case Game::Media::Type::Pictocouleur:
            return "pictocouleur";
        case Game::Media::Type::Pictomonochrome:
            return "pictomonochrome";
        default:
            return "unknown";
    }
}

std::string Api::toString(const Game::Language &language) {
    switch (language) {
        case Game::Language::EN:
            return "en";
        case Game::Language::FR:
            return "fr";
        case Game::Language::ES:
            return "es";
        case Game::Language::PT:
            return "pt";
        default:
            return "unknown";
    }
}

std::string Api::toString(const Game::Country &region) {
    switch (region) {
        case Game::Country::DE:
            return "de";
        case Game::Country::ASI:
            return "asi";
        case Game::Country::AU:
            return "au";
        case Game::Country::BR:
            return "br";
        case Game::Country::BG:
            return "bg";
        case Game::Country::CA:
            return "ca";
        case Game::Country::CL:
            return "cl";
        case Game::Country::CN:
            return "cn";
        case Game::Country::AME:
            return "ame";
        case Game::Country::KR:
            return "kr";
        case Game::Country::CUS:
            return "cus";
        case Game::Country::DK:
            return "dk";
        case Game::Country::SP:
            return "sp";
        case Game::Country::EU:
            return "eu";
        case Game::Country::FI:
            return "fi";
        case Game::Country::FR:
            return "fr";
        case Game::Country::GR:
            return "gr";
        case Game::Country::HU:
            return "hu";
        case Game::Country::IL:
            return "il";
        case Game::Country::IT:
            return "it";
        case Game::Country::JP:
            return "jp";
        case Game::Country::KW:
            return "kw";
        case Game::Country::WOR:
            return "wor";
        case Game::Country::MOR:
            return "mor";
        case Game::Country::NO:
            return "no";
        case Game::Country::NZ:
            return "nz";
        case Game::Country::OCE:
            return "oce";
        case Game::Country::NL:
            return "nl";
        case Game::Country::PE:
            return "pe";
        case Game::Country::PL:
            return "pl";
        case Game::Country::PT:
            return "pt";
        case Game::Country::CZ:
            return "cz";
        case Game::Country::UK:
            return "uk";
        case Game::Country::RU:
            return "ru";
        case Game::Country::SS:
            return "ss";
        case Game::Country::SK:
            return "sk";
        case Game::Country::SE:
            return "se";
        case Game::Country::TW:
            return "tw";
        case Game::Country::TR:
            return "tr";
        case Game::Country::US:
            return "us";
        case Game::Country::ALL:
            return "all";
        default:
            return "unknown";
    }
}

Game::Media::Type Api::toMedia(const std::string &type) {

    if (type == "sstitle") { return Game::Media::Type::SSTitle; }
    else if (type == "ss") { return Game::Media::Type::SS; }
    else if (type == "screenshot") { return Game::Media::Type::Screenshot; }
    else if (type == "fanart") { return Game::Media::Type::Fanart; }
    else if (type == "video") { return Game::Media::Type::Video; }
    else if (type == "marquee") { return Game::Media::Type::Marquee; }
    else if (type == "screenmarquee") { return Game::Media::Type::ScreenMarquee; }
    else if (type == "screenmarqueesmall") { return Game::Media::Type::ScreenMarqueeSmall; }
    else if (type == "themehs") { return Game::Media::Type::ThemeHs; }
    else if (type == "manuel") { return Game::Media::Type::Manuel; }
    else if (type == "flyer") { return Game::Media::Type::Flyer; }
    else if (type == "steamgrid") { return Game::Media::Type::SteamGrid; }
    else if (type == "wheel") { return Game::Media::Type::Wheel; }
    else if (type == "wheel-hd") { return Game::Media::Type::WheelHD; }
    else if (type == "wheel-carbon") { return Game::Media::Type::WheelCarbon; }
    else if (type == "wheel-steel") { return Game::Media::Type::WheelSteel; }
    else if (type == "box-2D") { return Game::Media::Type::Box2D; }
    else if (type == "box-2D-side") { return Game::Media::Type::Box2DSide; }
    else if (type == "box-2D-back") { return Game::Media::Type::Box2DBack; }
    else if (type == "box-texture") { return Game::Media::Type::BoxTexture; }
    else if (type == "box-2D-side") { return Game::Media::Type::Box2DSide; }
    else if (type == "box-3D") { return Game::Media::Type::Box3D; }
    else if (type == "box-scan") { return Game::Media::Type::BoxScan; }
    else if (type == "support-texture") { return Game::Media::Type::SupportTexture; }
    else if (type == "box-2D-side") { return Game::Media::Type::Box2DSide; }
    else if (type == "bezel-4-3") { return Game::Media::Type::Bezel43; }
    else if (type == "bezel-16-9") { return Game::Media::Type::Bezel169; }
    else if (type == "bezel-16-10") { return Game::Media::Type::Bezel1610; }
    else if (type == "mixrbv1") { return Game::Media::Type::Mixrbv1; }
    else if (type == "mixrbv2") { return Game::Media::Type::Mixrbv2; }
    else if (type == "pictoliste") { return Game::Media::Type::Pictoliste; }
    else if (type == "pictocouleur") { return Game::Media::Type::Pictocouleur; }
    else if (type == "pictomonochrome") { return Game::Media::Type::Pictomonochrome; }
    else { return Game::Media::Type::Unknow; }
}

Game::Country Api::toCountry(const std::string &country) {
    if (country == "de") { return Game::Country::US; }
    else if (country == "asi") { return Game::Country::ASI; }
    else if (country == "au") { return Game::Country::AU; }
    else if (country == "br") { return Game::Country::BR; }
    else if (country == "bg") { return Game::Country::BG; }
    else if (country == "ca") { return Game::Country::CA; }
    else if (country == "cl") { return Game::Country::CL; }
    else if (country == "cn") { return Game::Country::CN; }
    else if (country == "ame") { return Game::Country::AME; }
    else if (country == "kr") { return Game::Country::KR; }
    else if (country == "cus") { return Game::Country::CUS; }
    else if (country == "dk") { return Game::Country::DK; }
    else if (country == "sp") { return Game::Country::SP; }
    else if (country == "eu") { return Game::Country::EU; }
    else if (country == "fi") { return Game::Country::FI; }
    else if (country == "fr") { return Game::Country::FR; }
    else if (country == "gr") { return Game::Country::GR; }
    else if (country == "hu") { return Game::Country::HU; }
    else if (country == "il") { return Game::Country::IL; }
    else if (country == "it") { return Game::Country::IL; }
    else if (country == "jp") { return Game::Country::JP; }
    else if (country == "kw") { return Game::Country::KW; }
    else if (country == "wor") { return Game::Country::WOR; }
    else if (country == "mor") { return Game::Country::MOR; }
    else if (country == "no") { return Game::Country::NO; }
    else if (country == "nz") { return Game::Country::NZ; }
    else if (country == "oce") { return Game::Country::OCE; }
    else if (country == "nl") { return Game::Country::NL; }
    else if (country == "pe") { return Game::Country::PE; }
    else if (country == "pl") { return Game::Country::PL; }
    else if (country == "pt") { return Game::Country::PT; }
    else if (country == "cz") { return Game::Country::CZ; }
    else if (country == "uk") { return Game::Country::UK; }
    else if (country == "ru") { return Game::Country::RU; }
    else if (country == "ss") { return Game::Country::SS; }
    else if (country == "sk") { return Game::Country::SK; }
    else if (country == "se") { return Game::Country::SE; }
    else if (country == "tw") { return Game::Country::TW; }
    else if (country == "tr") { return Game::Country::TR; }
    else if (country == "us") { return Game::Country::US; }
    else if (country == "all") { return Game::Country::ALL; }
    else return Game::Country::UNKNOWN;
}

Game::Language Api::toLanguage(const std::string &language) {
    if (language == "en") { return Game::Language::EN; }
    else if (language == "fr") { return Game::Language::FR; }
    else if (language == "es") { return Game::Language::ES; }
    else if (language == "pt") { return Game::Language::PT; }
    else if (language == "all") { return Game::Language::ALL; }
    else return Game::Language::UNKNOWN;
}
