//
// Created by cpasjuste on 13/12/2019.
//

#ifndef SSCRAP_SS_SYSTEM_H
#define SSCRAP_SS_SYSTEM_H

#include <string>
#include <vector>
#include <tinyxml2.h>

namespace ss_api {

    class SystemList {
    public:

        struct System {
            System() = default;

            struct Names {
                Names() = default;

                std::string eu;
                std::string nom_us;
                std::string recalbox;
                std::string retropie;
                std::string launchbox;
                std::string hyperspin;
                std::string common;
            };

            std::string id;
            std::string parentid;
            Names names;
            std::string extensions;
            std::string company;
            std::string type;
            std::string startdate;
            std::string enddate;
            std::string romtype;
            std::string supporttype;
        };

        SystemList() = default;

        SystemList(const std::string &ssid, const std::string &sspassword, int retryDelay = 10);

        System findById(const std::string &id);

        static bool parseSystem(System *system, tinyxml2::XMLNode *systemNode);

        std::vector<System> systems;
        int http_error = 0;
    };
}

#endif //SSCRAP_SS_SYSTEM_H
