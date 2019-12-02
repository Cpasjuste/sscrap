//
// Created by cpasjuste on 16/09/2019.
//

#ifndef SSCRAP_SS_IO_H
#define SSCRAP_SS_IO_H

#include <string>
#include <vector>

namespace ss_api {
    class Io {
    public:
        static std::vector<std::string> getDirList(const std::string &path,
                                                   const std::string &ext = "zip");

        static void makedir(const std::string &path);

        static bool exist(const std::string &file);
    };
}

#endif //SSCRAP_SS_IO_H
