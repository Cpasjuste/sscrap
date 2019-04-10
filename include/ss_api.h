//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_API_H
#define SS_API_H

#include <json-c/json.h>

#include "ss_curl.h"
#include "ss_jeu.h"
#include "ss_user.h"

#define SS_TIMEOUT 10

namespace ss_api {

    class Api {

    public:

        enum Region {
            ALL, DE, ASI, AU, BR, BG, CA, CL,
            CN, AME, KR, CUS, DK, SP, EU,
            FI, FR, GR, HU, IL, IT, JP,
            KW, WOR, MOR, NO, NZ, OCE,
            NL, PE, PL, PT, CZ, UK, RU,
            SS, SK, SE, TW, TR, US
        };

        class JeuRecherche {
        public:
            User ssuser;
            std::vector<Jeu> jeux;
            std::string json;

            bool save(const std::string &dstPath);
        };

        class JeuInfos {
        public:
            User ssuser;
            Jeu jeu;
            std::string json;

            bool save(const std::string &dstPath);
        };

        explicit Api(const std::string &devid, const std::string &devpassword,
                     const std::string &softname);

        JeuRecherche jeuRecherche(const std::string &recherche, const std::string &systemeid,
                                  const std::string &ssid = "", const std::string &sspassword = "");

        JeuRecherche jeuRecherche(const std::string &srcPath);

        JeuInfos jeuInfos(const std::string &crc, const std::string &md5, const std::string &sha1,
                          const std::string &systemeid, const std::string &romtype,
                          const std::string &romnom, const std::string &romtaille, const std::string &gameid,
                          const std::string &ssid = "", const std::string &sspassword = "");

        JeuInfos jeuInfos(const std::string &srcPath);

        std::vector<Jeu::Media> getMedia(const Jeu &jeu, const Jeu::Media::Type &type, const Region &region = ALL);

        int download(const Jeu::Media &media, const std::string &dstPath);

        static std::string mediaTypeToString(const Jeu::Media::Type &type);

        static std::string regionToString(const Region &region);

    private:

        JeuRecherche parseJeuRecherche(const std::string &jsonData);

        JeuInfos parseJeuInfos(const std::string &jsonData);

        Jeu parseJeu(json_object *root);

        User parseUser(json_object *root);

        json_object *getJsonObject(json_object *root, const std::string &key);

        std::string getJsonString(json_object *root, const std::string &key);

        std::string devid;
        std::string devpassword;
        std::string softname;

        Curl curl;
    };
}

#endif //SCREENSCRAP_SS_API_H
