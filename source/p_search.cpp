//
// Created by cpasjuste on 29/03/19.
//

#include <cstring>
#include <string>
#include <algorithm>
#include <curl/curl.h>
#include "p_search.h"
#include "p_curl.h"

using namespace pscrap;

Search::Search(const std::string &key, const std::string &name, const std::string &lang, Type type) {

    api_key = key;
    language = lang;
    search = name;
    std::replace(search.begin(), search.end(), '.', ' ');
    std::replace(search.begin(), search.end(), '_', ' ');
    std::replace(search.begin(), search.end(), '-', ' ');

    CURL *curl = curl_easy_init();
    search = curl_easy_escape(curl, search.c_str(), (int) search.length());
    curl_easy_cleanup(curl);
}

int Search::get(int *http_code) {

    url = "https://api.themoviedb.org/3/search/movie?api_key="
          + api_key + "&language=" + language + "&query=" + search
          + "&page=1&include_adult=false";
    //printf("Search::get: %s\n", url.c_str());

    data = Curl::getString(url, http_code);
    if (data.empty()) {
        printf("Search::get: error: data is empty\n");
        return -1;
    }

    parseMovieRoot(data);

    while (total_results == 0) {

        size_t pos = search.rfind("%20");
        if (pos == std::string::npos) {
            break;
        }

        search = search.substr(0, pos);
        url = "https://api.themoviedb.org/3/search/movie?api_key="
              + api_key + "&language=" + language + "&query=" + search
              + "&page=1&include_adult=false";

        data = Curl::getString(url, http_code);
        if (data.empty()) {
            printf("Search::get: error: data is empty\n");
            return -1;
        }

        parseMovieRoot(data);
    }

    return 0;
}

int Search::load(const std::string &srcPath) {

    long size = 0;
    FILE *fp = fopen(srcPath.c_str(), "rb");
    if (!fp) {
        printf("Search::load: error: fopen failed\n");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    if (size <= 0) {
        printf("Search::load: error: ftell failed\n");
        fclose(fp);
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    data = std::string((unsigned long) size, '\0');
    fread(&data[0], sizeof(char), (size_t) size, fp);
    fclose(fp);

    if (data.empty()) {
        printf("Search::load: error: data is empty\n");
        return -1;
    }

    parseMovieRoot(data);

    return 0;
}

int Search::save(const std::string &dstPath) {

    FILE *fp = fopen(dstPath.c_str(), "wb");
    if (!fp) {
        printf("Search::save: error: fopen failed\n");
        return -1;
    }

    fwrite(data.c_str(), sizeof(char), data.length(), fp);
    fclose(fp);

    return 0;
}

void Search::parseMovieRoot(const std::string &jsonData) {

    json_object *root = json_tokener_parse(jsonData.c_str());
    json_object_object_foreach(root, key, obj) {
        enum json_type type = json_object_get_type(obj);
        switch (type) {
            case json_type_int:
                if (strcmp(key, "page") == 0) {
                    page = json_object_get_int(obj);
                } else if (strcmp(key, "total_results") == 0) {
                    total_results = json_object_get_int(obj);
                } else if (strcmp(key, "total_pages") == 0) {
                    total_pages = json_object_get_int(obj);
                }
                break;
            case json_type_array:
                if (strcmp(key, "results") == 0) {
                    parseMovie(obj);
                }
                break;

            default:
                break;
        }
    }
}

void Search::parseMovie(json_object *root) {

    int count = json_object_array_length(root);

    for (int i = 0; i < count; i++) {

        json_object *array = json_object_array_get_idx(root, i);
        enum json_type type = json_object_get_type(array);

        if (type == json_type_object) {
            Movie movie{};
            json_object_object_foreach(array, key, obj) {
                type = json_object_get_type(obj);
                switch (type) {
                    case json_type_boolean:
                        if (strcmp(key, "video") == 0) {
                            movie.video = (bool) json_object_get_boolean(obj);
                        } else if (strcmp(key, "adult") == 0) {
                            movie.adult = (bool) json_object_get_boolean(obj);
                        }
                        break;
                    case json_type_double:
                        if (strcmp(key, "vote_average") == 0) {
                            movie.vote_average = json_object_get_double(obj);
                        } else if (strcmp(key, "popularity") == 0) {
                            movie.popularity = json_object_get_double(obj);
                        }
                        break;
                    case json_type_int:
                        if (strcmp(key, "vote_count") == 0) {
                            movie.vote_count = json_object_get_int(obj);
                        } else if (strcmp(key, "id") == 0) {
                            movie.id = json_object_get_int(obj);
                        }
                        break;
                    case json_type_string:
                        if (strcmp(key, "title") == 0) {
                            movie.title = json_object_get_string(obj);
                        } else if (strcmp(key, "poster_path") == 0) {
                            movie.poster_path = json_object_get_string(obj);
                        } else if (strcmp(key, "original_language") == 0) {
                            movie.original_language = json_object_get_string(obj);
                        } else if (strcmp(key, "original_title") == 0) {
                            movie.original_title = json_object_get_string(obj);
                        } else if (strcmp(key, "backdrop_path") == 0) {
                            movie.backdrop_path = json_object_get_string(obj);
                        } else if (strcmp(key, "overview") == 0) {
                            movie.overview = json_object_get_string(obj);
                        } else if (strcmp(key, "release_date") == 0) {
                            movie.release_date = json_object_get_string(obj);
                        }
                        break;
                    case json_type_object:
                        break;
                    case json_type_array:
                        if (strcmp(key, "results") == 0) {
                            parseMovie(obj);
                        }
                        break;
                    default:
                        break;
                }
            }
            movies.push_back(movie);
        }
    }
}
