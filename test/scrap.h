//
// Created by cpasjuste on 21/11/2019.
//

#ifndef SSCRAP_SCRAP_H
#define SSCRAP_SCRAP_H

#include <pthread.h>

#ifndef _MSC_VER

#include <semaphore.h>

#endif

#include "args.h"

class Scrap {

public:

    struct MissFile {
        MissFile(const std::string &n, const std::string &p,
                 const std::string &z, const std::string &r) {
            name = n;
            path = p;
            zipCrc = z;
            romCrc = r;
        }

        std::string name;
        std::string path;
        std::string zipCrc;
        std::string romCrc;
    };

    explicit Scrap(const ArgumentParser &parser);

    void run();

    ArgumentParser args;
    std::string usr;
    std::string pwd;
    std::string romPath;
    ss_api::MediasGameList mediasGameList;
    ss_api::SystemList systemList;
    ss_api::GameList gameList;
    std::vector<std::string> filesList;
    std::vector<MissFile> missList;
    ss_api::User user;
    int filesCount = 0;
    bool mediasClone = false;
    pthread_t threads[15]{};
    pthread_mutex_t mutex;
};

#endif //SSCRAP_SCRAP_H
