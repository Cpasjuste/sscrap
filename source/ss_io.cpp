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

std::vector<Io::File> Io::getDirList(const std::string &path, bool recursive,
                                     const std::vector<std::string> &filters) {

    std::vector<Io::File> files;
    struct dirent *ent;
    DIR *dir;

    if (!path.empty()) {
        if ((dir = opendir(path.c_str())) != nullptr) {
            while ((ent = readdir(dir)) != nullptr) {

                // skip "hidden" files
                if (ent->d_name[0] == '.') {
                    continue;
                }

                File file = {ent->d_name, path + "/" + ent->d_name};
                file.size = getSize(file.path);

                if (ent->d_type == DT_DIR) {
                    if (!filters.empty()) {
                        for (const auto &filter : filters) {
                            if (file.name.find(filter) != std::string::npos) {
                                files.push_back(file);
                                break;
                            }
                        }
                    } else {
                        files.push_back(file);
                    }
                    if (recursive) {
                        std::vector<Io::File> subFiles = getDirList(file.path, true, filters);
                        for (const auto &x : subFiles) {
                            files.push_back(x);
                        }
                    }
                } else if (ent->d_type == DT_REG) {
                    if (!filters.empty()) {
                        for (const auto &filter : filters) {
                            if (file.name.find(filter) != std::string::npos) {
                                files.push_back(file);
                                break;
                            }
                        }
                    } else {
                        files.push_back(file);
                    }
                } else {
                    continue;
                }
            }
            closedir(dir);
        }
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
