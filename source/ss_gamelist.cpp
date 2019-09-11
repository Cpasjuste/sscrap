//
// Created by cpasjuste on 11/09/2019.
//

//
// Created by cpasjuste on 09/09/2019.
// Parse emulationstation gamelist.xml
//

#include <algorithm>
#include "ss_api.h"
#include "ss_gamelist.h"

using namespace tinyxml2;
using namespace ss_api;

bool GameList::load(const std::string &xmlPath) {

    tinyxml2::XMLDocument doc;

    XMLError e = doc.LoadFile(xmlPath.c_str());
    if (e != XML_SUCCESS) {
        printf("GameList: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return false;
    }

    XMLNode *pRoot = doc.FirstChildElement("gameList");
    if (!pRoot) {
        printf("GameList: incorrect xml format (\'gameList\' tag not found)");
        return false;
    }

    XMLNode *gameNode = pRoot->FirstChildElement("game");
    if (!gameNode) {
        printf("GameList: incorrect xml format (\'game\' tag not found)");
        return false;
    }

    while (gameNode) {
        Game game{};
        // parse id
        if (gameNode->ToElement()->Attribute("id")) {
            game.id = gameNode->ToElement()->Attribute("id");
        }
        // parse source
        if (gameNode->ToElement()->Attribute("source")) {
            game.source = gameNode->ToElement()->Attribute("source");
        }
        // parse path
        XMLElement *element = gameNode->FirstChildElement("path");
        if (element && element->GetText()) {
            game.path = element->GetText();
        }
        // parse name
        element = gameNode->FirstChildElement("name");
        if (element && element->GetText()) {
            game.names.push_back({"wor", element->GetText()});
        }
        // parse description
        element = gameNode->FirstChildElement("desc");
        if (element && element->GetText()) {
            game.synopsis.push_back({"wor", element->GetText()});
        }
        // parse image
        element = gameNode->FirstChildElement("image");
        if (element && element->GetText()) {
            game.medias.push_back({"sstitle", "", element->GetText(), "wor", "", "", "", "", ""});
        }
        // parse thumbnail
        element = gameNode->FirstChildElement("thumbnail");
        if (element && element->GetText()) {
            game.medias.push_back({"screenshot", "", element->GetText(), "wor", "", "", "", "", ""});
        }
        // parse rating
        element = gameNode->FirstChildElement("rating");
        if (element && element->GetText()) {
            char *endPtr = nullptr;
            double d = strtod(element->GetText(), &endPtr);
            if (!*endPtr) {
                game.rating = (float) d;
            }
        }
        // parse release date
        element = gameNode->FirstChildElement("releasedate");
        if (element && element->GetText()) {
            Game::Date date{"wor", element->GetText()};
            game.dates.emplace_back(date);
            // add release date (year for now) to releasedateList for later filtering
            auto p = std::find(releasedateList.begin(), releasedateList.end(), date.text);
            if (p == releasedateList.end()) {
                releasedateList.emplace_back(date.text);
            }
        }
        // parse developer
        element = gameNode->FirstChildElement("developer");
        if (element && element->GetText()) {
            game.developer.text = element->GetText();
            // add developer to developerList for later filtering
            auto p = std::find(developerList.begin(), developerList.end(), game.developer.text);
            if (p == developerList.end()) {
                developerList.emplace_back(game.developer.text);
            }
        }
        // parse editor
        element = gameNode->FirstChildElement("publisher");
        if (element && element->GetText()) {
            game.editor.text = element->GetText();
            // add editor to editorList for later filtering
            auto p = std::find(editorList.begin(), editorList.end(), game.editor.text);
            if (p == editorList.end()) {
                editorList.emplace_back(game.editor.text);
            }
        }
        // parse genre
        element = gameNode->FirstChildElement("genre");
        if (element && element->GetText()) {
            Game::Genre genre{"", element->GetText(), "", std::vector<Game::Genre::Name>()};
            game.genres.emplace_back(genre);
            // add genre to genreList for later filtering
            auto p = std::find(genreList.begin(), genreList.end(), genre.main);
            if (p == genreList.end()) {
                genreList.emplace_back(genre.main);
            }
        }
        // parse players
        element = gameNode->FirstChildElement("players");
        if (element && element->GetText()) {
            game.players = element->GetText();
            // add players to playersList for later filtering
            auto p = std::find(playersList.begin(), playersList.end(), game.players);
            if (p == playersList.end()) {
                playersList.emplace_back(game.players);

            }
        }

        // add game to game list
        games.emplace_back(game);
        // move to next node (game)
        gameNode = gameNode->NextSibling();
    }

    // sort lists
    std::sort(releasedateList.begin(), releasedateList.end(), sortByName);
    std::sort(developerList.begin(), developerList.end(), sortByName);
    std::sort(editorList.begin(), editorList.end(), sortByName);
    std::sort(genreList.begin(), genreList.end(), sortByName);
    std::sort(playersList.begin(), playersList.end(), sortByName);
    // default lists values
    if (!releasedateList.empty() && releasedateList.at(0) != "All") {
        releasedateList.insert(releasedateList.begin(), "All");
    }
    if (!developerList.empty() && developerList.at(0) != "All") {
        developerList.insert(developerList.begin(), "All");
    }
    if (!editorList.empty() && editorList.at(0) != "All") {
        editorList.insert(editorList.begin(), "All");
    }
    if (!genreList.empty() && genreList.at(0) != "All") {
        genreList.insert(genreList.begin(), "All");
    }
    if (!playersList.empty() && playersList.at(0) != "All") {
        playersList.insert(playersList.begin(), "All");
    }

    return true;
}

bool GameList::save(const std::string &path) {

    XMLDocument doc;

    XMLDeclaration *dec = doc.NewDeclaration();
    doc.InsertFirstChild(dec);

    XMLNode *pRoot = doc.NewElement("gameList");
    doc.InsertEndChild(pRoot);

    for (const auto &game : games) {
        // game element
        XMLElement *gameElement = doc.NewElement("game");
        gameElement->SetAttribute("id", game.id.c_str());
        gameElement->SetAttribute("source", game.source.c_str());
        // path
        XMLElement *child = doc.NewElement("path");
        child->SetText(game.path.c_str());
        gameElement->InsertEndChild(child);
        // name
        child = doc.NewElement("name");
        if (!game.names.empty()) {
            child->SetText(game.names.at(0).text.c_str());
        }
        gameElement->InsertEndChild(child);
        // desc
        child = doc.NewElement("desc");
        if (!game.synopsis.empty()) {
            child->SetText(game.synopsis.at(0).text.c_str());
        }
        gameElement->InsertEndChild(child);
        // image
        child = doc.NewElement("image");
        if (!game.medias.empty()) {
            std::vector<Game::Media> medias = Api::getMedia(game, Game::Media::Type::SSTitle, Api::Country::WOR);
            if (!medias.empty()) {
                child->SetText(medias.at(0).url.c_str());
            }
        }
        gameElement->InsertEndChild(child);
        // thumbnail
        child = doc.NewElement("thumbnail");
        if (!game.medias.empty()) {
            std::vector<Game::Media> medias = Api::getMedia(game, Game::Media::Type::Screenshot, Api::Country::WOR);
            if (!medias.empty()) {
                child->SetText(medias.at(0).url.c_str());
            }
        }
        gameElement->InsertEndChild(child);
        // releasedate
        child = doc.NewElement("releasedate");
        if (!game.dates.empty()) {
            child->SetText(game.dates.at(0).text.c_str());
        }
        gameElement->InsertEndChild(child);
        // developer
        child = doc.NewElement("developer");
        child->SetText(game.developer.text.c_str());
        gameElement->InsertEndChild(child);
        // publisher
        child = doc.NewElement("publisher");
        child->SetText(game.editor.text.c_str());
        gameElement->InsertEndChild(child);
        // genre
        child = doc.NewElement("genre");
        if (!game.genres.empty()) {
            child->SetText(game.genres.at(0).main.c_str());
        }
        gameElement->InsertEndChild(child);
        // players
        child = doc.NewElement("players");
        child->SetText(game.players.c_str());
        gameElement->InsertEndChild(child);
        // add game element
        pRoot->InsertEndChild(gameElement);
    }

    XMLError e = doc.SaveFile(path.c_str());
    if (e != XML_SUCCESS) {
        printf("GameList::save: %s\n", tinyxml2::XMLDocument::ErrorIDToName(e));
        return false;
    }

    return true;
}

std::vector<Game> GameList::filter(const std::string &date, const std::string &developer, const std::string &publisher,
                                   const std::string &genre, const std::string &players) {
    std::vector<Game> newGameList;
    /*
    std::copy_if(games.begin(), games.end(), std::back_inserter(newGameList),
                 [date, developer, publisher, genre, players](const Jeu &game) {
                     return (date == "All" || game.releasedate >= date)
                            && (developer == "All" || game.developer == developer)
                            && (publisher == "All" || game.publisher == publisher)
                            && (genre == "All" || game.genre == genre)
                            && (players == "All" || game.players == players);
                 });
    */
    return newGameList;
}

void GameList::clear() {
    games.clear();
}

bool GameList::sortByName(const std::string &g1, const std::string &g2) {
    return strcasecmp(g1.c_str(), g2.c_str()) <= 0;
}
