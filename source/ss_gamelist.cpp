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
        // screenscraper / emulationstation compat
        if (gameNode->ToElement()->Attribute("id")) {
            game.id = gameNode->ToElement()->Attribute("id");
        }
        // screenscraper
        if (gameNode->ToElement()->Attribute("romid")) {
            game.romid = gameNode->ToElement()->Attribute("romid");
        }
        // emulationstation compat
        if (gameNode->ToElement()->Attribute("source")) {
            game.source = gameNode->ToElement()->Attribute("source");
        }
        // emulationstation compat
        XMLElement *element = gameNode->FirstChildElement("path");
        if (element && element->GetText()) {
            game.path = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("notgame");
        if (element && element->GetText()) {
            game.notgame = element->GetText();
        }
        // screenscraper (prioritise screenscraper format)
        element = gameNode->FirstChildElement("names");
        if (element) {
            XMLNode *node = element->FirstChildElement("name");
            while (node) {
                Game::Name gameName{};
                if (node->ToElement()->Attribute("country")) {
                    gameName.country = node->ToElement()->Attribute("country");
                }
                if (node->ToElement()->Attribute("text")) {
                    gameName.text = node->ToElement()->Attribute("text");
                }
                game.names.emplace_back(gameName);
                node = node->NextSibling();
            }
        } else {
            // emulationstation compat (use emulationstation format)
            element = gameNode->FirstChildElement("name");
            if (element && element->GetText()) {
                game.names.push_back({"wor", element->GetText()});
            }
        }
        // screenscraper
        element = gameNode->FirstChildElement("countries");
        if (element) {
            XMLNode *node = element->FirstChildElement("shortname");
            while (node) {
                if (node->ToElement()->GetText()) {
                    game.countries.emplace_back(node->ToElement()->GetText());
                }
                node = node->NextSibling();
            }
        }
        // screenscraper
        element = gameNode->FirstChildElement("cloneof");
        if (element && element->GetText()) {
            game.cloneof = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("systemeid");
        if (element && element->GetText()) {
            game.systemeid = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("systemename");
        if (element && element->GetText()) {
            game.systemename = element->GetText();
        }
        // screenscraper (prioritise screenscraper format)
        element = gameNode->FirstChildElement("synopses");
        if (element) {
            XMLNode *node = element->FirstChildElement("synopsis");
            while (node) {
                Game::Synopsis synopsis{};
                if (node->ToElement()->Attribute("language")) {
                    synopsis.language = node->ToElement()->Attribute("language");
                }
                if (node->ToElement()->Attribute("text")) {
                    synopsis.text = node->ToElement()->Attribute("text");
                }
                game.synopses.emplace_back(synopsis);
                node = node->NextSibling();
            }
        } else {
            // emulationstation compat (use emulationstation format)
            element = gameNode->FirstChildElement("desc");
            if (element && element->GetText()) {
                game.synopses.push_back({"wor", element->GetText()});
            }
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

        // screenscraper (prioritise screenscraper format)
        element = gameNode->FirstChildElement("dates");
        if (element) {
            XMLNode *node = element->FirstChildElement("date");
            while (node) {
                Game::Date date{};
                if (node->ToElement()->Attribute("country")) {
                    date.country = node->ToElement()->Attribute("country");
                }
                if (node->ToElement()->Attribute("text")) {
                    date.text = node->ToElement()->Attribute("text");
                }
                game.dates.emplace_back(date);
                node = node->NextSibling();
            }
        } else {
            // emulationstation compat (use emulationstation format)
            element = gameNode->FirstChildElement("releasedate");
            if (element && element->GetText()) {
                Game::Date date{"wor", element->GetText()};
                game.dates.emplace_back(date);
            }
        }
        // parse developer
        element = gameNode->FirstChildElement("developer");
        if (element && element->GetText()) {
            game.developer.text = element->GetText();
        }
        // parse editor
        element = gameNode->FirstChildElement("publisher");
        if (element && element->GetText()) {
            game.editor.text = element->GetText();
        }
        // screenscraper (prioritise screenscraper format)
        element = gameNode->FirstChildElement("genres");
        if (element) {
            XMLNode *node = element->FirstChildElement("genre");
            while (node) {
                Game::Genre genre{};
                if (node->ToElement()->Attribute("id")) {
                    genre.id = node->ToElement()->Attribute("id");
                }
                if (node->ToElement()->Attribute("main")) {
                    genre.main = node->ToElement()->Attribute("main");
                }
                if (node->ToElement()->Attribute("parentid")) {
                    genre.parentid = node->ToElement()->Attribute("parentid");
                }
                XMLNode *childNode = node->FirstChildElement("names");
                while (childNode) {
                    Game::Genre::Name name{};
                    if (childNode->ToElement()->Attribute("language")) {
                        name.language = node->ToElement()->Attribute("language");
                    }
                    if (childNode->ToElement()->Attribute("text")) {
                        name.text = node->ToElement()->Attribute("text");
                    }
                    genre.names.emplace_back(name);
                    childNode = childNode->NextSibling();
                }
                game.genres.emplace_back(genre);
                node = node->NextSibling();
            }
        } else {
            // emulationstation compat (use emulationstation format)
            element = gameNode->FirstChildElement("genre");
            if (element && element->GetText()) {
                Game::Genre genre{"", element->GetText(), "", std::vector<Game::Genre::Name>()};
                game.genres.emplace_back(genre);
            }
        }
        // screenscraper
        element = gameNode->FirstChildElement("players");
        if (element && element->GetText()) {
            game.players = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("topstaff");
        if (element && element->GetText()) {
            game.topstaff = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("rotation");
        if (element && element->GetText()) {
            game.rotation = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("resolution");
        if (element && element->GetText()) {
            game.resolution = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("inputs");
        if (element && element->GetText()) {
            game.inputs = element->GetText();
        }
        // screenscraper
        element = gameNode->FirstChildElement("colors");
        if (element && element->GetText()) {
            game.colors = element->GetText();
        }

        // add game to game list
        games.emplace_back(game);
        // move to next node (game)
        gameNode = gameNode->NextSibling();
    }

    // TODO: build filtering lists
    /*
    // add players to playersList for later filtering
    auto p = std::find(playersList.begin(), playersList.end(), game.players);
    if (p == playersList.end()) {
        playersList.emplace_back(game.players);

    }
    */
    // sort lists
    std::sort(releasedateList.begin(), releasedateList.end(), sortByName);
    std::sort(developerList.begin(), developerList.end(), sortByName);
    std::sort(editorList.begin(), editorList.end(), sortByName);
    std::sort(genreList.begin(), genreList.end(), sortByName);
    std::sort(playersList.begin(), playersList.end(), sortByName);
    // default lists values
    if (releasedateList.empty() || releasedateList.at(0) != "All") {
        releasedateList.insert(releasedateList.begin(), "All");
    }
    if (developerList.empty() || developerList.at(0) != "All") {
        developerList.insert(developerList.begin(), "All");
    }
    if (editorList.empty() || editorList.at(0) != "All") {
        editorList.insert(editorList.begin(), "All");
    }
    if (genreList.empty() || genreList.at(0) != "All") {
        genreList.insert(genreList.begin(), "All");
    }
    if (playersList.empty() || playersList.at(0) != "All") {
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
        // screenscraper / emulationstation
        XMLElement *gameElement = doc.NewElement("game");
        gameElement->SetAttribute("id", game.id.c_str());
        gameElement->SetAttribute("romid", game.romid.c_str());
        gameElement->SetAttribute("source", game.source.c_str());
        // emulationstation
        XMLElement *elem = doc.NewElement("path");
        elem->SetText(game.path.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("notgame");
        elem->SetText(game.notgame.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *names = doc.NewElement("names");
        for (const auto &name : game.names) {
            XMLElement *n = doc.NewElement("name");
            n->SetAttribute("country", name.country.c_str());
            n->SetAttribute("text", name.text.c_str());
            names->InsertEndChild(n);
        }
        gameElement->InsertEndChild(names);
        // emulationstation
        elem = doc.NewElement("name");
        if (!game.names.empty()) {
            elem->SetText(game.getName(Game::Country::WOR).text.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *countries = doc.NewElement("countries");
        for (const auto &country : game.countries) {
            XMLElement *n = doc.NewElement("shortname");
            n->SetText(country.c_str());
            countries->InsertEndChild(n);
        }
        gameElement->InsertEndChild(countries);
        // screenscraper
        elem = doc.NewElement("cloneof");
        if (!game.cloneof.empty()) {
            elem->SetText(game.cloneof.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("systemeid");
        if (!game.systemeid.empty()) {
            elem->SetText(game.systemeid.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("systemename");
        if (!game.systemename.empty()) {
            elem->SetText(game.systemename.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *synopses = doc.NewElement("synopses");
        for (const auto &synopsis : game.synopses) {
            XMLElement *n = doc.NewElement("synopsis");
            n->SetAttribute("language", synopsis.language.c_str());
            n->SetAttribute("text", synopsis.text.c_str());
            synopses->InsertEndChild(n);
        }
        gameElement->InsertEndChild(synopses);
        // emulationstation
        elem = doc.NewElement("desc");
        if (!game.synopses.empty()) {
            elem->SetText(game.synopses.at(0).text.c_str());
            elem->SetText(game.getSynopsis(Game::Language::EN).text.c_str());
        }
        gameElement->InsertEndChild(elem);
        // image
        elem = doc.NewElement("image");
        Game::Media image = game.getMedia(Game::Media::Type::SS, Game::Country::WOR);
        if (!image.url.empty()) {
            elem->SetText(image.url.c_str());
        }
        gameElement->InsertEndChild(elem);
        // thumbnail
        elem = doc.NewElement("thumbnail");
        Game::Media thumbnail = game.getMedia(Game::Media::Type::Box3D, Game::Country::WOR);
        if (!thumbnail.url.empty()) {
            elem->SetText(thumbnail.url.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *dates = doc.NewElement("dates");
        for (const auto &date : game.dates) {
            XMLElement *n = doc.NewElement("date");
            n->SetAttribute("country", date.country.c_str());
            n->SetAttribute("text", date.text.c_str());
            dates->InsertEndChild(n);
        }
        gameElement->InsertEndChild(dates);
        // emulationstation
        elem = doc.NewElement("releasedate");
        if (!game.dates.empty()) {
            elem->SetText(game.dates.at(0).text.c_str());
        }
        gameElement->InsertEndChild(elem);
        // developer
        elem = doc.NewElement("developer");
        elem->SetText(game.developer.text.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper / emulationstation
        elem = doc.NewElement("publisher");
        elem->SetText(game.editor.text.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        XMLElement *genres = doc.NewElement("genres");
        for (const auto &genre : game.genres) {
            XMLElement *n = doc.NewElement("genre");
            n->SetAttribute("country", genre.id.c_str());
            n->SetAttribute("main", genre.main.c_str());
            n->SetAttribute("parentid", genre.parentid.c_str());
            XMLElement *namesElem = doc.NewElement("names");
            for (const auto &name : genre.names) {
                XMLElement *n2 = doc.NewElement("name");
                n2->SetAttribute("language", name.language.c_str());
                n2->SetAttribute("text", name.text.c_str());
                namesElem->InsertEndChild(n2);
            }
            n->InsertEndChild(namesElem);
            genres->InsertEndChild(n);
        }
        gameElement->InsertEndChild(genres);
        // emulationstation
        elem = doc.NewElement("genre");
        if (!game.genres.empty()) {
            elem->SetText(game.genres.at(0).main.c_str());
        }
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("players");
        elem->SetText(game.players.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("topstaff");
        elem->SetText(game.topstaff.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("rotation");
        elem->SetText(game.rotation.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("resolution");
        elem->SetText(game.resolution.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("inputs");
        elem->SetText(game.inputs.c_str());
        gameElement->InsertEndChild(elem);
        // screenscraper
        elem = doc.NewElement("colors");
        elem->SetText(game.colors.c_str());
        gameElement->InsertEndChild(elem);

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
                 [date, developer, publisher, genre, players](const Game &game) {
                     return (date == "All" || game.dates >= date)
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
