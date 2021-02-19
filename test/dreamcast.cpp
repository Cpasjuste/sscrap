//
// Created by cpasjuste on 16/02/2021.
//

#include <string>
#include <cstring>
#include "utility.h"
#include "dreamcast.h"

Dreamcast::IpHeader Dreamcast::getIpHeader(const std::string &path) {

    IpHeader header{};

    FILE *file = fopen(path.c_str(), "rb");
    if (!file) {
        printf("Dreamcast::getIpHeader: could not open file: \"%s\"\n", path.c_str());
        return header;
    }

    if (Utility::getExt(path) == "bin") {
        fseek(file, 16, SEEK_SET);
    }

    size_t read = fread(&header, 1, IP_HEADER_SIZE, file);
    fclose(file);
    if (read != IP_HEADER_SIZE) {
        printf("Dreamcast::getIpHeader: could not read file: \"%s\"\n", path.c_str());
        return header;
    }

    // TODO... (hint: no null terminating character)
    header.hardware_id[15] = '\0';
    header.disk_type[5] = '\0';
    header.disk_num[4] = '\0';
    header.release_date[7] = '\0';
    header.company[15] = '\0';
    // trim..
    for (int i = 126; i > 0; i--) {
        if (!isspace(header.name[i])) {
            header.name[i + 1] = '\0';
            break;
        }
    }

    return header;
}
