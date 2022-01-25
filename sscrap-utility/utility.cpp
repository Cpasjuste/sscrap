//
// Created by cpasjuste on 10/12/2019.
//

#include <cstring>
#include <algorithm>

#ifndef __VITA__

#include <minizip/unzip.h>

#endif

#include <ss_api.h>
#include <dirent.h>
#include "utility.h"

using namespace ss_api;

static hashwrapper *md5Wrapper;
static hashwrapper *sha1Wrapper;

std::string Utility::removeExt(const std::string &str) {
    size_t pos = str.find_last_of('.');
    if (pos != std::string::npos) {
        return str.substr(0, pos);
    } else {
        return str;
    }
}

int Utility::parseInt(const std::string &str, int defValue) {
    char *end = nullptr;
    long i = strtol(str.c_str(), &end, 10);
    if (end != nullptr) {
        return (int) i;
    }
    return defValue;
}

std::string Utility::getExt(const std::string &file) {

    char ext[3];

    if (file.length() < 4) {
        return "";
    }

    ext[0] = file[file.size() - 3];
    ext[1] = file[file.size() - 2];
    ext[2] = file[file.size() - 1];

    return ext;
}

std::string Utility::getRomCrc(const std::string &zipPath, std::vector<std::string> whiteList) {

    char *zipFileName, *data, buffer[16];
    memset(buffer, 0, 16);

    if (!Io::endsWith(zipPath, ".zip", false)) {
        return "";
    }

#ifndef __VITA__
    unzFile zip = unzOpen(zipPath.c_str());
    if (zip == nullptr) {
        SS_PRINT("could not open zip file for crc check (%s)\n", zipPath.c_str());
        return buffer;
    }

    if (unzGoToFirstFile(zip) == UNZ_OK) {
        do {
            if (unzOpenCurrentFile(zip) == UNZ_OK) {
                unz_file_info fileInfo;
                memset(&fileInfo, 0, sizeof(unz_file_info));
                if (unzGetCurrentFileInfo(zip, &fileInfo, nullptr, 0,
                                          nullptr, 0, nullptr, 0) == UNZ_OK) {
                    zipFileName = (char *) malloc((uLong) fileInfo.size_filename + 1);
                    unzGetCurrentFileInfo(zip, &fileInfo, zipFileName, fileInfo.size_filename + 1,
                                          nullptr, 0, nullptr, 0);
                    zipFileName[(uLong) fileInfo.size_filename] = '\0';
                    std::string ext = getExt(std::string(zipFileName));
                    if (whiteList.empty() || std::find(whiteList.begin(), whiteList.end(), ext) != whiteList.end()) {
                        data = (char *) malloc(fileInfo.uncompressed_size);
                        unzReadCurrentFile(zip, data, (unsigned int) fileInfo.uncompressed_size);
                        uLong crc = crc32(0L, (const Bytef *) data, fileInfo.uncompressed_size) & 0xffffffff;
                        free(data);
                        free(zipFileName);
                        unzClose(zip);
                        snprintf(buffer, 16, "%08lx", crc);
                        return buffer;
                    }
                    free(zipFileName);
                }
                unzCloseCurrentFile(zip);
            }
        } while (unzGoToNextFile(zip) == UNZ_OK);
    }

    unzClose(zip);
#endif
    return buffer;
}

std::string Utility::getFileCrc(const std::string &zipPath) {

    unsigned char buffer[BUFSIZ];
    char hex[16];
    size_t size;
    FILE *pFile;

    memset(hex, 0, 16);

#ifndef __VITA__
#ifdef _MSC_VER
    fopen_s(&pFile, zipPath.c_str(), "rb");
#else
    pFile = fopen(zipPath.c_str(), "rb");
#endif
    if (pFile == nullptr) {
        return hex;
    }

    uLong crc = crc32(0L, Z_NULL, 0);
    while ((size = fread(buffer, 1, BUFSIZ, pFile)) != 0) {
        crc = crc32(crc, buffer, size);
    }
    snprintf(hex, 16, "%08lx", crc);

    fclose(pFile);
#endif
    return hex;
}

#if 0
std::string Utility::getFileMd5(const std::string &path) {
    if (!md5Wrapper) {
        md5Wrapper = new md5wrapper();
        sha1Wrapper = new sha1wrapper();
    }
    return md5Wrapper->getHashFromFile(path);
}

std::string Utility::getFileSha1(const std::string &path) {
    if (!md5Wrapper) {
        md5Wrapper = new md5wrapper();
        sha1Wrapper = new sha1wrapper();
    }
    return sha1Wrapper->getHashFromFile(path);
}
#endif

Utility::ZipInfo Utility::getZipInfo(const std::string &path, const std::string &file) {

    ZipInfo info;
    info.name = file;
    std::string fullPath = path + "/" + file;

    if (!Io::exist(fullPath)) {
        return info;
    }

    if (!md5Wrapper) {
        md5Wrapper = new md5wrapper();
        sha1Wrapper = new sha1wrapper();
    }

    info.name = file;
    info.size = std::to_string(Io::getSize(fullPath));
    info.crc = getFileCrc(fullPath);
    info.md5 = md5Wrapper->getHashFromFile(fullPath);
    info.sha1 = sha1Wrapper->getHashFromFile(fullPath);

    return info;
}

std::string Utility::getZipInfoStr(const std::string &path, const std::string &file) {

    if (!md5Wrapper) {
        md5Wrapper = new md5wrapper();
        sha1Wrapper = new sha1wrapper();
    }

    ZipInfo info = getZipInfo(path, file);
    return info.name + "|" + info.size + "|" + info.serial + "|" + info.crc + "|" + info.md5 + "|" + info.sha1;
}

#if 0
void Utility::replace(std::string &str, const std::string &from, const std::string &to) {
    size_t start_pos = str.find(from);
    while (start_pos != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos = str.find(from);
    }
}

void Utility::printGame(const Game &game) {

    printf("\n===================================\n");
    Game::Name name = game.getName(Game::Country::SS);
    printf("nom (%s): %s (alternatives: %i)\n", Api::toString(name.country).c_str(), name.text.c_str(),
           (int) game.names.size() - 1);
    printf("available: %i\n", (int) game.available);
    printf("path: %s\n", game.path.c_str());
    printf("id: %s\n", std::to_string(game.id).c_str());
    printf("cloneof: %s\n", game.cloneOf.c_str());
    printf("system: %s (id: %s)\n", game.system.text.c_str(), std::to_string(game.system.id).c_str());
    printf("editor: %s (id: %s)\n", game.editor.text.c_str(), std::to_string(game.editor.id).c_str());
    printf("developer: %s (id: %s)\n", game.developer.text.c_str(), std::to_string(game.developer.id).c_str());
    printf("players: %s\n", game.players.c_str());
    printf("rating: %s\n", std::to_string(game.rating).c_str());
    printf("topstaff: %s\n", game.topStaff ? "true" : "false");
    printf("rotation: %s\n", std::to_string(game.rotation).c_str());
    printf("resolution: %s\n", game.resolution.c_str());
    Game::Synopsis synopsis = game.getSynopsis(Game::Language::EN);
    printf("synopsis (%s): %s\n", Api::toString(synopsis.language).c_str(), synopsis.text.c_str());
    Game::Date date = game.getDate(Game::Country::WOR);
    printf("date (%s): %s\n", Api::toString(date.country).c_str(), date.text.c_str());
    Game::Genre genre = game.getGenre(Game::Language::EN);
    printf("genre (%s): %s\n", Api::toString(genre.language).c_str(), genre.text.c_str());
    // print some medias
    Game::Media media = game.getMedia("sstitle", Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    media = game.getMedia("ss", Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
    media = game.getMedia("mixrbv2", Game::Country::WOR);
    printf("media (%s): %s\n", media.type.c_str(), media.url.c_str());
}
#endif
