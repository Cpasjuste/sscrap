//
// Created by cpasjuste on 29/03/19.
//

#ifndef PSCRAP_P_CURL_H
#define PSCRAP_P_CURL_H

#include <string>
#include <curl/curl.h>

namespace ss_api {

    class Curl {

    public:

        Curl();

        ~Curl();

        std::string getString(const std::string &url, int timeout, long *http_code);

        int getData(const std::string &url, const std::string &dstPath, int timeout, long *http_code);

        std::string escape(const std::string &url);

    private:

        CURL *curl = nullptr;
    };

}

#endif //PSCRAP_P_CURL_H