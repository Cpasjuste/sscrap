//
// Created by cpasjuste on 29/03/19.
//

#ifndef PSCRAP_P_MOVIE_H
#define PSCRAP_P_MOVIE_H

#include <string>
#include <vector>

namespace pscrap {

    class Movie {

    public:

        int getPoster(const std::string &dstPath, int width = 200);

        int getBackdrop(const std::string &dstPath, int width = 500);

        int vote_count;
        int id;
        bool video;
        double vote_average;
        std::string title;
        double popularity;
        std::string poster_path;
        std::string original_language;
        std::string original_title;
        std::vector<int> genre_ids;
        std::string backdrop_path;
        bool adult;
        std::string overview;
        std::string release_date;
    };
}

#endif //PSCRAP_P_MOVIE_H
