//
// Created by cpasjuste on 11/09/2019.
//

//
// Created by cpasjuste on 09/09/2019.
// Parse emulationstation gamelist.xml
//

#include <algorithm>
#include "ss_gamelist.h"

using namespace tinyxml2;
using namespace ss_api;

bool GameList::load(const std::string &xmlPath) {

    /*
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
        Jeu game;
        // parse id
        if (gameNode->ToElement()->Attribute("id")) {
            game.id = gameNode->ToElement()->Attribute("id");
        }
        // parse romid
        if (gameNode->ToElement()->Attribute("romid")) {
            game.romid = gameNode->ToElement()->Attribute("romid");
        }
        // parse notgame
        XMLElement *element = gameNode->FirstChildElement("notgame");
        if (element && element->GetText()) {
            game.notgame = element->GetText();
        }
        // parse names
        element = gameNode->FirstChildElement("names");
        if (element && element->GetText()) {
            game.nom = element->GetText();
        }
        // parse description
        element = gameNode->FirstChildElement("desc");
        if (element && element->GetText()) {
            game.desc = element->GetText();
        }
        // parse image
        element = gameNode->FirstChildElement("image");
        if (element && element->GetText()) {
            game.image = element->GetText();
        }
        // parse thumbnail
        element = gameNode->FirstChildElement("thumbnail");
        if (element && element->GetText()) {
            game.thumbnail = element->GetText();
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
            game.releasedate = element->GetText();
            // add release date (year for now) to releasedateList for later filtering
            auto p = std::find(releasedateList.begin(), releasedateList.end(), game.releasedate);
            if (p == releasedateList.end()) {
                releasedateList.emplace_back(game.releasedate);
            }
        }
        // parse developer
        element = gameNode->FirstChildElement("developer");
        if (element && element->GetText()) {
            game.developer = element->GetText();
            // add developer to developerList for later filtering
            auto p = std::find(developerList.begin(), developerList.end(), game.developer);
            if (p == developerList.end()) {
                developerList.emplace_back(game.developer);
            }
        }
        // parse publisher
        element = gameNode->FirstChildElement("publisher");
        if (element && element->GetText()) {
            game.publisher = element->GetText();
            // add publisher to publisherList for later filtering
            auto p = std::find(publisherList.begin(), publisherList.end(), game.publisher);
            if (p == publisherList.end()) {
                publisherList.emplace_back(game.publisher);
            }
        }
        // parse genre
        element = gameNode->FirstChildElement("genre");
        if (element && element->GetText()) {
            game.genre = element->GetText();
            // add genre to genreList for later filtering
            auto p = std::find(genreList.begin(), genreList.end(), game.genre);
            if (p == genreList.end()) {
                genreList.emplace_back(game.genre);
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
    std::sort(publisherList.begin(), publisherList.end(), sortByName);
    std::sort(genreList.begin(), genreList.end(), sortByName);
    std::sort(playersList.begin(), playersList.end(), sortByName);
    // default lists values
    releasedateList.insert(releasedateList.begin(), "All");
    developerList.insert(developerList.begin(), "All");
    publisherList.insert(publisherList.begin(), "All");
    genreList.insert(genreList.begin(), "All");
    playersList.insert(playersList.begin(), "All");

     */
    return true;
}

bool GameList::save(const std::string &path) {

    /*
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
        child->SetText(game.name.c_str());
        gameElement->InsertEndChild(child);
        // desc
        child = doc.NewElement("desc");
        child->SetText(game.desc.c_str());
        gameElement->InsertEndChild(child);
        // image
        child = doc.NewElement("image");
        child->SetText(game.image.c_str());
        gameElement->InsertEndChild(child);
        // thumbnail
        child = doc.NewElement("thumbnail");
        child->SetText(game.thumbnail.c_str());
        gameElement->InsertEndChild(child);
        // releasedate
        child = doc.NewElement("releasedate");
        child->SetText(game.releasedate.c_str());
        gameElement->InsertEndChild(child);
        // developer
        child = doc.NewElement("developer");
        child->SetText(game.developer.c_str());
        gameElement->InsertEndChild(child);
        // publisher
        child = doc.NewElement("publisher");
        child->SetText(game.publisher.c_str());
        gameElement->InsertEndChild(child);
        // genre
        child = doc.NewElement("genre");
        child->SetText(game.genre.c_str());
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
    */
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

bool GameList::sortByName(const std::string &g1, const std::string &g2) {
    return strcasecmp(g1.c_str(), g2.c_str()) <= 0;
}
