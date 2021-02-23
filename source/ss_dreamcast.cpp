//
// Created by cpasjuste on 23/02/2021.
//

#include <algorithm>
#include "ss_dreamcast.h"

using namespace ss_api;

static std::vector<Dreamcast::HeaderFix> headerFixList = {
        {"VIRTUA STRIKER 2 VER.2000.1", "Virtua Striker 2"},
        {"HEADHUNTER - SEGA - AMUZE - 2001", "Headhunter"},
        {"SHENMUE2 EU", "Shenmue 2"},
        {"STREET FIGHTER3 DOUBLE IMPACT", "Street Fighter 3 - Double Impact"},
        {"SUPER PUZZLE FIGHTER2X FOR MATCHING SERVICE", "Super Puzzle Fighter 2 X"},
        {"AERO DANCING TODOROKI TAICHOU NO HIMITSU DISK", "Aero Dancing - Todoroki Taichou No Himitsu Disc"}
};

Dreamcast::HeaderFix Dreamcast::getHeaderFix(const std::string &headerName) {

    auto it = std::find_if(headerFixList.begin(), headerFixList.end(), [headerName](const HeaderFix &headerFix) {
        return headerFix.headerName == headerName;
    });

    if (it != headerFixList.end()) {
        return *it;
    }

    return {};
}
