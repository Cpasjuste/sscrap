//
// Created by cpasjuste on 05/04/19.
//

#ifndef SCREENSCRAP_S_SYSTEM_H
#define SCREENSCRAP_S_SYSTEM_H

#if 0
#include <string>
#include <vector>

class System {

public:

    class Media {
    public:
        std::string parent;
        std::string type;
        std::string region;
        std::string crc;
        std::string md5;
        std::string sha1;
        std::string format;
    };

    int id;
    int parent_id;
    std::string name_eu;
    std::string name_us;
    std::vector<std::string> extensions;
    std::string company;
    std::string type;
    std::string date_start;
    std::string date_end;
    std::string rom_type;
    std::string support_type;
    std::vector<Media> medias;
};

#endif
#endif //SCREENSCRAP_S_SYSTEM_H

