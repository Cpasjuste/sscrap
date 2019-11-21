//
// Created by cpasjuste on 16/09/2019.
//

#include <dirent.h>
#include <sys/stat.h>
#include "ss_io.h"

using namespace ss_api;

std::vector<std::string> Io::getDirList(const std::string &path) {

    std::vector<std::string> files;
    struct dirent *ent;
    DIR *dir;

    if ((dir = opendir(path.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            // skip "."
            if (ent->d_name[0] == '.' || ent->d_type != DT_REG) {
                continue;
            }
            files.emplace_back(ent->d_name);
        }
        closedir(dir);
    }
    return files;
}

void Io::makedir(const std::string &path) {
    mkdir(path.c_str(), 0755);
}
