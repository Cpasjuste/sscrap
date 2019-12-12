//
// Created by cpasjuste on 21/11/2019.
//

#ifndef SSCRAP_SCRAP_H
#define SSCRAP_SCRAP_H

#ifndef _MSC_VER
#include <semaphore.h>
#endif
#include "args.h"

class Scrap {

public:

    explicit Scrap(const ArgumentParser &parser);

    void run();

    ArgumentParser args;
    std::string usr;
    std::string pwd;
    ss_api::Game::Language language = ss_api::Game::Language::EN;
    std::string romPath;
    ss_api::MediasGameList mediasGameList;
    ss_api::GameList gameList;
    std::vector<std::string> filesList;
    std::vector<std::string> missList;
    ss_api::User user;
    int filesCount = 0;
    bool mediasClone = false;
#ifdef _MSC_VER
    HANDLE threads[15];
    HANDLE mutex;
#define pthread_mutex_lock(x) WaitForSingleObject(x, INFINITE)
#define pthread_mutex_unlock(x) ReleaseMutex(x)
#else
    pthread_t threads[15]{};
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
};

#endif //SSCRAP_SCRAP_H
