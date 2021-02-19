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
        class File {
        public:
            std::string name;
            std::string path;
            size_t size;
        };

        static std::vector<File> getDirList(
                const std::string &path, bool recursive,
                const std::vector<std::string> &filters = {".zip"});

        static void makedir(const std::string &path);

        static bool exist(const std::string &file);

        static size_t getSize(const std::string &file);

        static void delay(int seconds);
    };
}

#endif //SSCRAP_SS_IO_H
