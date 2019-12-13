//
// Created by cpasjuste on 29/03/19.
//

#include "ss_api.h"
#include "scrap.h"
#include "args.h"
#include "utility.h"

using namespace ss_api;

static Scrap *scrap;
static int retryDelay = 10;

void fixFbnClone(Game *game, const GameList &fbnGameList) {

    // screenscraper game "cloneof" is set, continue
    if (game->isClone()) {
        return;
    }

    // mame/fbneo: search fbn list for zip name (path), as game name may differ
    const std::string zipName = game->path;
    auto fbaGame = std::find_if(fbnGameList.games.begin(), fbnGameList.games.end(), [zipName](const Game &g) {
        return zipName == g.path && g.isClone();
    });
    // screenscraper game not found in fbneo dat, or is not a clone continue...
    if (fbaGame == fbnGameList.games.end()) {
        return;
    }
    // fix screenscraper cloneof !
    game->cloneof = (*fbaGame).cloneof + ".zip";
}

static void *scrap_thread(void *ptr) {

    int tid = *((int *) ptr);
    Game fbnGame;
    GameList fbnGameList;
    bool isFbNeoSystem = false;

    // if a custom sscrap custom id is set (fbneo console games),
    // we need to use "FinalBurn Neo" databases "description" as rom name
    // and map to correct screenscraper systemid
    std::string sid = scrap->args.get("-sid");
    if (sid == "75" || sid == "750" || sid == "751" || sid == "752" || sid == "753"
        || sid == "754" || sid == "755" || sid == "756" || sid == "757" || sid == "758" || sid == "759") {
        isFbNeoSystem = true;
        if (sid == "75") {
            // mame/fbneo
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Arcade only).dat");
        } else if (sid == "750") {
            // colecovision
            sid = "48";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ColecoVision only).dat");
        } else if (sid == "751") {
            // game gear
            sid = "21";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Game Gear only).dat");
        } else if (sid == "752") {
            // master system
            sid = "2";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Master System only).dat");
        } else if (sid == "753") {
            // megadrive
            sid = "1";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Megadrive only).dat");
        } else if (sid == "754") {
            // msx
            sid = "113";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, MSX 1 Games only).dat");
        } else if (sid == "755") {
            // pc engine
            sid = "31";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, PC-Engine only).dat");
        } else if (sid == "756") {
            // sega sg-1000
            sid = "109";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, Sega SG-1000 only).dat");
        } else if (sid == "757") {
            // super grafx
            sid = "105";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, SuprGrafx only).dat");
        } else if (sid == "758") {
            // turbo grafx
            sid = "31";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, TurboGrafx16 only).dat");
        } else if (sid == "759") {
            // zx spectrum
            sid = "76";
            fbnGameList.append("databases/FinalBurn Neo (ClrMame Pro XML, ZX Spectrum Games only).dat");
        }
    }

    while (!scrap->filesList.empty()) {

        pthread_mutex_lock(&scrap->mutex);
        std::string file = scrap->filesList.at(0);
        scrap->filesList.erase(scrap->filesList.begin());
        int filesSize = (int) scrap->filesList.size();
        pthread_mutex_unlock(&scrap->mutex);

        GameInfo gameInfo;
        std::string searchType = "None";

        // first, search by zip crc
        std::string path = scrap->romPath + "/" + file;
        std::string crc = Utility::getZipCrc(path);
        if (!crc.empty()) {
            gameInfo = GameInfo(crc, "", "", sid, "", file,
                                "", "", scrap->usr, scrap->pwd, retryDelay);
            if (gameInfo.http_error == 0) {
                searchType = "zip_crc";
            } else if (gameInfo.http_error == 430) {
                fprintf(stderr, KYEL "NOK: thread[%i] => Quota reached for today... "
                                "See \' https://www.screenscraper.fr/\' if you want to support "
                                "screenscraper and maximise your quota!\n" KRAS, tid);
                break;
            }
        }
        SS_PRINT("zip crc: %s (%s), res = %i\n", file.c_str(), crc.c_str(), gameInfo.http_error);

        // next, try by rom crc if not mame / fbneo arcade (multiple roms in zip...)
        if (gameInfo.http_error != 0 && sid != "75") {
            path = scrap->romPath + "/" + file;
            crc = Utility::getRomCrc(path);
            if (!crc.empty()) {
                gameInfo = GameInfo(crc, "", "", sid, "", file,
                                    "", "", scrap->usr, scrap->pwd, retryDelay);
                if (gameInfo.http_error == 0) {
                    searchType = "rom_crc";
                } else if (gameInfo.http_error == 430) {
                    fprintf(stderr, KYEL "NOK: thread[%i] => Quota reached for today... "
                                    "See \' https://www.screenscraper.fr/\' if you want to support "
                                    "screenscraper and maximise your quota!\n" KRAS, tid);
                    break;
                }
            }
            SS_PRINT("rom crc: %s (%s), res = %i\n", file.c_str(), crc.c_str(), gameInfo.http_error);
        }

        // fbneo consoles zip names doesn't match standard consoles zip names
        // this will also help fbneo arcade games if not found by zip name
        if (isFbNeoSystem) {
            fbnGame = fbnGameList.findByPath(file);
        }

        // now try with zip name
        if (gameInfo.http_error != 0) {
            std::string name = (isFbNeoSystem && sid != "75") ? fbnGame.getName().text + ".zip" : file;
            gameInfo = GameInfo("", "", "", sid, "rom", name,
                                "", "", scrap->usr, scrap->pwd, retryDelay);
            if (gameInfo.http_error == 0) {
                searchType = "zip_name";
            }
            if (isFbNeoSystem && sid != "75") {
                gameInfo.game.path = file;
            }
            SS_PRINT("zip name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
        }

        // finally, try a game search (jeuRecherche)
        if (gameInfo.http_error != 0) {
            // the rom is not know by screenscraper, try to find the game with a game search (jeuRecherche)
            std::string name = isFbNeoSystem ? fbnGame.getName().text : file;
            GameSearch search = GameSearch(name, sid, scrap->usr, scrap->pwd, retryDelay);
            SS_PRINT("search name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
            if (!search.games.empty()) {
                auto game = std::find_if(search.games.begin(), search.games.end(), [sid](const Game &game) {
                    return game.system.id == sid;
                });
                if (game != search.games.end()) {
                    gameInfo = GameInfo("", "", "", "", "", file,
                                        "", (*game).id, scrap->usr, scrap->pwd, retryDelay);
                    if (gameInfo.http_error == 0) {
                        searchType = "search";
                    }
                    SS_PRINT("search name (romid): %s, res = %i\n", name.c_str(), gameInfo.http_error);
                }
            } else {
                // last try, remove "(xxx)"
                size_t pos = name.find_first_of('(');
                if (pos != std::string::npos) {
                    name = name.substr(0, pos - 1);
                    search = GameSearch(name, sid, scrap->usr, scrap->pwd, retryDelay);
                    SS_PRINT("search name: %s, res = %i\n", name.c_str(), gameInfo.http_error);
                    if (!search.games.empty()) {
                        auto game = std::find_if(search.games.begin(), search.games.end(), [sid](const Game &game) {
                            return game.system.id == sid;
                        });
                        if (game != search.games.end()) {
                            gameInfo = GameInfo("", "", "", "", "", file,
                                                "", (*game).id, scrap->usr, scrap->pwd, retryDelay);
                            if (gameInfo.http_error == 0) {
                                searchType = "search";
                            }
                            SS_PRINT("search name (romid): %s, res = %i\n", name.c_str(), gameInfo.http_error);
                        }
                    }
                }
            }
        }

        if (isFbNeoSystem) {
            fixFbnClone(&gameInfo.game, fbnGameList);
        }

        if (gameInfo.http_error == 0) {
            if (scrap->args.exist("-dlm") && (!scrap->mediasClone && !gameInfo.game.isClone())) {
                for (const auto &mediaType : scrap->mediasGameList.medias) {
                    if (!scrap->args.exist(mediaType.nameShort)) {
                        continue;
                    }
                    Game::Media media = gameInfo.game.getMedia(mediaType.nameShort, Game::Country::SS);
                    if (media.url.empty()) {
                        continue;
                    }
                    std::string name = file.substr(0, file.find_last_of('.') + 1) + media.format;
                    path = scrap->romPath + "/media/" + media.type + "/";
                    if (!Io::exist(path)) {
                        Io::makedir(path);
                    }
                    path += name;
                    if (Io::exist(path)) {
                        continue;
                    }
                    int res = media.download(path);
                    while (res == 429) {
                        pthread_mutex_lock(&scrap->mutex);
                        fprintf(stderr,
                                KYEL "NOK: thread[%i] => maximum requests per minute reached... retrying in %i seconds\n" KRAS,
                                tid, retryDelay);
                        pthread_mutex_unlock(&scrap->mutex);
                        Io::delay(retryDelay);
                        res = media.download(path);
                    }
                }
            }

            pthread_mutex_lock(&scrap->mutex);
            printf(KGRE "[%i/%i] OK: %s => %s (%s) (%s)\n" KRAS,
                   scrap->filesCount - filesSize, scrap->filesCount,
                   file.c_str(), gameInfo.game.getName().text.c_str(),
                   gameInfo.game.system.text.c_str(), searchType.c_str());
            scrap->gameList.games.emplace_back(gameInfo.game);
            pthread_mutex_unlock(&scrap->mutex);
        } else {
            pthread_mutex_lock(&scrap->mutex);
            // game not found, but add it to the list with default values
            if (isFbNeoSystem) {
                scrap->gameList.games.emplace_back(fbnGame);
            } else {
                Game game;
                game.names.emplace_back(Api::toString(Game::Country::WOR), file);
                game.path = file;
                scrap->gameList.games.emplace_back(game);
            }
            fprintf(stderr, KRED "[%i/%i] NOK: %s (%i)\n" KRAS,
                    scrap->filesCount - filesSize, scrap->filesCount,
                    isFbNeoSystem ? fbnGame.getName().text.c_str() : file.c_str(), gameInfo.http_error);
            scrap->missList.emplace_back(file);
            pthread_mutex_unlock(&scrap->mutex);
        }
    }

    return nullptr;
}

Scrap::Scrap(const ArgumentParser &parser) {

    args = parser;
    usr = args.get("-u");
    pwd = args.get("-p");
    mediasClone = args.exist("-dlmc");

    if (args.exist("-language")) {
        language = Api::toLanguage(args.get("-language"));
        if (language == Game::Language::UNKNOWN) {
            fprintf(stderr, KRED "ERROR: language not found: %s, available languages: en, fr, es, pt\n" KRAS,
                    args.get("-language").c_str());
            return;
        }
        printf("language: %s\n", Api::toString(language).c_str());
    }
}

void Scrap::run() {

    if (args.exist("-r")) {
        romPath = args.get("-r");
        filesList = Io::getDirList(romPath);
        filesCount = (int) filesList.size();
        if (filesList.empty()) {
            fprintf(stderr, KRED "ERROR: no files found in rom path\n" KRAS);
            return;
        }

        user = User(usr, pwd);
        mediasGameList = MediasGameList(usr, pwd, retryDelay);

        if (args.exist("-dlm")) {
            Io::makedir(romPath + "/media");
        }

        pthread_mutex_init(&mutex, nullptr);

        int maxThreads = user.getMaxThreads();
        for (int i = 0; i < maxThreads; i++) {
            // yes, there's a minor memory leak there...
            int *tid = (int *) malloc(sizeof(*tid));
            *tid = i;
            pthread_create(&threads[i], nullptr, scrap_thread, (void *) tid);
        }

        for (int i = 0; i < maxThreads; i++) {
            pthread_join(threads[i], nullptr);
        }

        pthread_mutex_destroy(&mutex);

        if (!gameList.games.empty() && args.exist("-sx")) {
            Game::Language lang = Game::Language::EN;
            if (args.exist("fr")) {
                lang = Game::Language::FR;
            } else if (args.exist("es")) {
                lang = Game::Language::ES;
            } else if (args.exist("pt")) {
                lang = Game::Language::PT;
            }
            GameList::Format fmt = args.exist("screenscraper") ?
                                   GameList::Format::ScreenScrapper : GameList::Format::EmulationStation;
            std::vector<std::string> mediaList;
            if (fmt == GameList::ScreenScrapper) {
                for (const auto &mediaType : mediasGameList.medias) {
                    if (args.exist(mediaType.nameShort)) {
                        mediaList.emplace_back(mediaType.nameShort);
                    }
                }
            } else {
                mediaList.emplace_back(args.get("-i"));
                mediaList.emplace_back(args.get("-t"));
                mediaList.emplace_back(args.get("-v"));
            }
            gameList.save(romPath + "/gamelist.xml", lang, fmt, mediaList);
        }
        printf(KGRE "\n==========\nALL DONE\n==========\n" KRAS);
        printf(KGRE "found %zu/%i games\n" KRAS, gameList.games.size() - missList.size(), filesCount);
        if (!missList.empty()) {
            printf(KGRE "%zu was not found:\n" KRAS, missList.size());
            for (const auto &file : missList) {
                printf(KRED "%s, " KRAS, file.c_str());
            }
        }
        printf("\n");
    } else if (args.exist("-ml")) {
        mediasGameList = MediasGameList(usr, pwd, retryDelay);
        printf(KGRE "\nAvailable screenscraper medias types:\n\n" KRAS);
        for (const auto &media : mediasGameList.medias) {
            printf(KGRE "\t%s " KRAS "(type: %s, category: %s)\n",
                   media.nameShort.c_str(), media.type.c_str(), media.category.c_str());
        }
        printf("\n");
    } else {
        printf("\n");
        printf("usage: sscrap -u <screenscraper_user> -p <screenscraper_password> [options] [options_es]\n");
        printf("\n\toptions:\n");
        printf("\t\t-d                             enable debug output\n");
        printf("\t\t-ml                            list available screenscraper medias types and exit\n");
        printf("\t\t-r <roms_path>                 path to roms files to scrap\n");
        printf("\t\t-sid <system_id>               screenscraper system id to scrap\n");
        printf("\t\t-dlm <mediaType1 mediaType2>   download given medias types\n");
        printf("\t\t-dlmc                          download medias for clones\n");
        printf("\t\t-sx <language> <format>        save xml with given output language and format to romspath\n");
        printf("\n\toptions_es:\n");
        printf("\t\t-i <mediaType>                 use given media type for image\n");
        printf("\t\t-t <mediaType>                 use given media type for thumbnail\n");
        printf("\t\t-v <mediaType>                 use given media type for video\n");
        printf("\n\tsscrap supported output languages\n");
        printf("\t\ten: english, default\n");
        printf("\t\tfr: french\n");
        printf("\t\tes: spanish\n");
        printf("\t\tpt: portuguese\n");
        printf("\n\tsscrap supported output formats\n");
        printf("\t\temulationstation: emulationstation gamelist.xml, default\n");
        printf("\t\tscreenscraper: screenscraper gamelist.xml\n");
        printf("\n\tsscrap customs systemid (fbneo)\n");
        printf("\t\t750: ColecoVision\n");
        printf("\t\t751: Game Gear\n");
        printf("\t\t752: Master System\n");
        printf("\t\t753: Megadrive\n");
        printf("\t\t754: MSX 1\n");
        printf("\t\t755: NEC PC-Engine\n");
        printf("\t\t756: Sega SG-1000\n");
        printf("\t\t757: NEC Super Grafx\n");
        printf("\t\t758: NEC Turbo Grafx\n");
        printf("\t\t759: ZX Spectrum\n");
        printf("\n");
        printf("examples:\n\n");
        printf("\tscrap mame/fbneo system, download \'mixrbv2\' for \'image\', \'box-3D\' for \'thumbnail\' and \'video\' "
               "for \'video\' and output xml as emulationstation format in english:\n");
        printf("\t\tsscrap -u user -p password -r /roms -sid 75 -dlm mixrbv2 box-3D video -i mixrbv2 -t box-3D -v video -sx\n\n");
        printf("\tscrap mame/fbneo system, download \'mixrbv2\' and \'video\' medias types and output xml as screenscraper format in french (for pFBN):\n");
        printf("\t\tsscrap -u user -p password -r /roms -sid 75 -dlm mixrbv2 video -sx fr screenscraper\n");
        printf("\n");
    }
}

int main(int argc, char **argv) {

    ArgumentParser args(argc, argv);

    // setup screenscraper api
#ifdef _MSC_VER
    Api::ss_devid = "";
    Api::ss_devpassword = "";
#else
    Api::ss_devid = SS_DEV_ID;
    Api::ss_devpassword = SS_DEV_PWD;
#endif
    Api::ss_softname = "sscrap";
    ss_debug = args.exist("-d");
#ifndef NDEBUG
    ss_debug = true;
#endif

    scrap = new Scrap(args);
    scrap->run();

    return 0;
}
