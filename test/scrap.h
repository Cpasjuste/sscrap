//
// Created by cpasjuste on 21/11/2019.
//

#ifndef SSCRAP_SCRAP_H
#define SSCRAP_SCRAP_H

#include <semaphore.h>
#include "args.h"

class Scrap {

public:

    explicit Scrap(const ArgumentParser &parser);

    void run();

    ArgumentParser args;
    std::string user;
    std::string pwd;
    ss_api::Game::Language language = ss_api::Game::Language::EN;
    std::string romPath;
    ss_api::Api::GameList gameList;
    std::vector<std::string> fileList;
    std::vector<std::string> missList;
    bool mediaClone = false;
    pthread_t threads[15];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
};

#endif //SSCRAP_SCRAP_H
