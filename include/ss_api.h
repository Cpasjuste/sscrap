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

        explicit Api(const std::string &devid, const std::string &devpassword,
                     const std::string &softname);

        JeuRecherche jeuRecherche(const std::string &recherche, const std::string &systemeid,
                                  const std::string &ssid = "test", const std::string &sspassword = "test");


    private:

        JeuRecherche parseJeuRecherche(const std::string &jsonData);

        json_object *getJsonObject(json_object *root, const std::string &key);

        std::string getJsonString(json_object *root, const std::string &key);

        std::string devid;
        std::string devpassword;
        std::string softname;

        Curl curl;
    };
}

#endif //SCREENSCRAP_SS_API_H
