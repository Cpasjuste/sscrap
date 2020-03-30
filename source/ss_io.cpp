//
// Created by cpasjuste on 16/09/2019.
//

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ss_io.h"

#ifdef __WINDOWS__
#include <windows.h>
#define mkdir(x, y) mkdir(x)
#elif __VITA__
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/stat.h>
#define mkdir(x, y) sceIoMkdir(x, 06)
#endif

using namespace ss_api;

std::vector<std::string> Io::getDirList(const std::string &path, const std::string &ext) {

    std::vector<std::string> files;
    struct stat st{};
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

    return files;
}

void Io::makedir(const std::string &path) {
    mkdir(path.c_str(), 0755);
}

bool Io::exist(const std::string &file) {
    struct stat st{};
    return (stat(file.c_str(), &st) == 0);
}

size_t Io::getSize(const std::string &file) {
    struct stat st{};
    if (stat(file.c_str(), &st) != 0) {
        return 0;
    }
    return (size_t) st.st_size;
}

void Io::delay(int seconds) {
#ifdef __VITA__
    sceKernelDelayThread(seconds * 1000000);
#else
    sleep(seconds);
#endif
}
