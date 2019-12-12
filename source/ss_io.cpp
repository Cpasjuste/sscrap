//
// Created by cpasjuste on 16/09/2019.
//

#ifdef _MSC_VER
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include "ss_io.h"

#ifdef __WINDOWS__
#include <windows.h>
#define mkdir(x, y) mkdir(x)
#endif

using namespace ss_api;

std::vector<std::string> Io::getDirList(const std::string &path, const std::string &ext) {

    std::vector<std::string> files;

#ifdef _MSC_VER

#else
    struct dirent *ent;
    DIR *dir;
#ifdef __WINDOWS__
    std::string filePath;
#endif

    if ((dir = opendir(path.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            // skip "."
            if (ent->d_name[0] == '.') {
                continue;
            }
#ifdef __WINDOWS__
            filePath = path + "\\" + ent->d_name;
            if(GetFileAttributes(filePath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
                printf("skip: %s\n", ent->d_name);
                continue;
            }
#else
            if (ent->d_type != DT_REG) {
                continue;
            }
#endif
            std::string file = ent->d_name;
            if (!ext.empty()) {
                if (file.rfind('.') != std::string::npos
                    && file.substr(file.find_last_of('.') + 1) == ext) {
                    files.emplace_back(file);
                }
            } else {
                files.emplace_back(file);
            }
        }
        closedir(dir);
    }
#endif
    return files;
}

void Io::makedir(const std::string &path) {
#ifdef _MSC_VER
    CreateDirectoryA(path.c_str(), nullptr);
#else
    mkdir(path.c_str(), 0755);
#endif
}

bool Io::exist(const std::string &file) {
    struct stat st{};
    return (stat(file.c_str(), &st) == 0);
}

void Io::delay(int seconds) {
#ifdef _MSC_VER
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif
}
