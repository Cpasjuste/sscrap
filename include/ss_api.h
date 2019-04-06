//
// Created by cpasjuste on 05/04/19.
//

#ifndef SS_API_H
#define SS_API_H

#include <json-c/json.h>

#include "ss_curl.h"
#include "ss_jeu.h"
#include "ss_user.h"

namespace ss_api {

    class Api {

    public:

        struct JeuRecherche {
            User ssuser;
            std::vector<Jeu> jeux;
        };

        struct JeuInfos {
            User ssuser;
            Jeu jeu;
        };

        explicit Api(const std::string &devid, const std::string &devpassword,
                     const std::string &softname);

        JeuRecherche jeuRecherche(const std::string &recherche, const std::string &systemeid,
                                  const std::string &ssid = "", const std::string &sspassword = "");

        JeuInfos jeuInfos(const std::string &crc, const std::string &md5, const std::string &sha1,
                          const std::string &systemeid, const std::string &romtype,
                          const std::string &romnom, const std::string &romtaille, const std::string &gameid,
                          const std::string &ssid = "", const std::string &sspassword = "");

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
