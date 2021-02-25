//
// Created by cpasjuste on 23/02/2021.
//

#include <algorithm>
#include "ss_dreamcast.h"

using namespace ss_api;

static std::vector<Dreamcast::HeaderFix> headerFixList = {
        {"VIRTUA STRIKER 2 VER.2000.1",                   "Virtua Striker 2"},
        {"HEADHUNTER - SEGA - AMUZE - 2001",              "Headhunter"},
        {"SHENMUE2 EU",                                   "Shenmue 2"},
        {"STREET FIGHTER3 DOUBLE IMPACT",                 "Street Fighter 3 - Double Impact"},
        {"SUPER PUZZLE FIGHTER2X FOR MATCHING SERVICE",   "Super Puzzle Fighter 2 X"},
        {"AERO DANCING TODOROKI TAICHOU NO HIMITSU DISK", "Aero Dancing - Todoroki Taichou No Himitsu Disc"},
        {"ESPN INTERNATIONAL TRACK AND FIELD",            "Ganbare Nippon! Olympic 2000"},
        {"POWER STONE 2 USA",                             "Power Stone 2"},
        {"POWER STONE 2 PAL",                             "Power Stone 2"},
        {"SKIES OF ARCADIA DISC 1",                       "Skies of Arcadia"},
        {"SKIES OF ARCADIA DISC 2",                       "Skies of Arcadia"},
        {"SKIES OF ARCADIA DISC1",                        "Skies of Arcadia"},
        {"SKIES OF ARCADIA DISC2",                        "Skies of Arcadia"},
        {"ROADSTERS RC7",                                 "Roadsters"},
        {"RESIDENT EVIL3",                                "Resident Evil 3 - Nemesis"},
        {"ROYAL RUMBLE",                                  "Wwf Royal Rumble"},
        {"VF3TB",                                         "Virtua Fighter 3tb"},
        {"STREET FIGHTER ALPHA3 SAIKYO-DOJO",             "Street Fighter Alpha 3"},
        {"STREET FIGHTER3 DOUBLE IMPACT",                 "Street Fighter 3 - Double Impact"},
        {"ESL",                                           "European Super League"},
        {"PSO VER.2",                                     "Phantasy Star Online Ver.2"}
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
