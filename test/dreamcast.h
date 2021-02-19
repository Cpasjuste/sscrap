//
// Created by cpasjuste on 16/02/2021.
//

#ifndef SSCRAP_DREAMCAST_H
#define SSCRAP_DREAMCAST_H

#include <vector>

#define IP_HEADER_SIZE 256
#define IP_HEADER_ITEMS 18

class Dreamcast {

public:

    // https://mc.pp.se/dc/ip0000.bin.html#periph
    typedef struct IpHeader {
        char hardware_id[16];
        char maker_id[16];
        char ks[5];
        char disk_type[6];
        char disk_num[5];
        char country_codes[8];
        char ctrl[4];
        char dev[1];
        char vga[1];
        char wince[1];
        char padding1[1];
        char product_number[10];
        char product_version[6];
        char release_date[8];
        char padding2[8];
        char boot_filename[16];
        char company[16];
        char name[128];
    } ip_header_t;

    static IpHeader getIpHeader(const std::string &path);
};

#endif //SSCRAP_DREAMCAST_H
