//
// Created by cpasjuste on 29/03/19.
//

#ifndef PSCRAP_P_CURL_H
#define PSCRAP_P_CURL_H

#include <string>

namespace pscrap {

    class Curl {

    public:

        static std::string getString(const std::string &url, int *http_code);

        static int getData(const std::string &url, const std::string &dstPath, int *http_code);
    };

}

#endif //PSCRAP_P_CURL_H
