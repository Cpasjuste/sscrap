//
// Created by cpasjuste on 05/04/19.
//

#include <cstdarg>
#include "ss_api.h"

using namespace ss_api;

std::string Api::ss_devid;
std::string Api::ss_devpassword;
std::string Api::ss_softname;
bool ss_debug = false;

std::string Api::getXmlAttrStr(tinyxml2::XMLElement *element, const std::string &name) {

    if (element == nullptr || element->Attribute(name.c_str()) == nullptr) {
        return "";
    }

    return element->Attribute(name.c_str());
}

int Api::getXmlAttrInt(tinyxml2::XMLElement *element, const std::string &name) {

    std::string attr = getXmlAttrStr(element, name);
    return parseInt(attr);
}

long Api::getXmlAttrLong(tinyxml2::XMLElement *element, const std::string &name) {

    std::string attr = getXmlAttrStr(element, name);
    return parseLong(attr);
}

bool Api::getXmlAttrBool(tinyxml2::XMLElement *element, const std::string &name) {

    std::string attr = getXmlAttrStr(element, name);
    return parseBool(attr);
}

Game::Country Api::getXmlAttrCountry(tinyxml2::XMLElement *element, const std::string &name) {

    std::string attr = getXmlAttrStr(element, name);
    return toCountry(attr);
}

Game::Language Api::getXmlAttrLang(tinyxml2::XMLElement *element, const std::string &name) {

    std::string attr = getXmlAttrStr(element, name);
    return toLanguage(attr);
}

std::string Api::getXmlTextStr(tinyxml2::XMLElement *element) {

    if (element == nullptr || element->GetText() == nullptr) {
        return "";
    }

    return element->GetText();
}

int Api::getXmlTextInt(tinyxml2::XMLElement *element) {

    std::string text = getXmlTextStr(element);
    return parseInt(text);
}

bool Api::getXmlTextBool(tinyxml2::XMLElement *element) {

    std::string text = getXmlTextStr(element);
    return parseBool(text);
}

Game::Country Api::getXmlTextCountry(tinyxml2::XMLElement *element) {

    std::string text = getXmlTextStr(element);
    return toCountry(text);
}

Game::Language Api::getXmlTextLang(tinyxml2::XMLElement *element) {

    std::string text = getXmlTextStr(element);
    return toLanguage(text);
}

tinyxml2::XMLElement *Api::addXmlElement(tinyxml2::XMLDocument *doc, tinyxml2::XMLElement *parent,
                                         const std::string &name, const std::string &value) {

    if (parent == nullptr) {
        return nullptr;
    }

    if (!value.empty()) {
        tinyxml2::XMLElement *element = doc->NewElement(name.c_str());
        element->SetText(value.c_str());
        parent->InsertEndChild(element);
        return element;
    }

    return nullptr;
}

std::string Api::toString(const Game::Language &language) {
    switch (language) {
        case Game::Language::EN:
            return "en";
        case Game::Language::FR:
            return "fr";
        case Game::Language::ES:
            return "es";
        case Game::Language::PT:
            return "pt";
        default:
            return "unk";
    }
}

std::string Api::toString(const Game::Country &region) {
    switch (region) {
        case Game::Country::DE:
            return "de";
        case Game::Country::ASI:
            return "asi";
        case Game::Country::AU:
            return "au";
        case Game::Country::BR:
            return "br";
        case Game::Country::BG:
            return "bg";
        case Game::Country::CA:
            return "ca";
        case Game::Country::CL:
            return "cl";
        case Game::Country::CN:
            return "cn";
        case Game::Country::AME:
            return "ame";
        case Game::Country::KR:
            return "kr";
        case Game::Country::CUS:
            return "cus";
        case Game::Country::DK:
            return "dk";
        case Game::Country::SP:
            return "sp";
        case Game::Country::EU:
            return "eu";
        case Game::Country::FI:
            return "fi";
        case Game::Country::FR:
            return "fr";
        case Game::Country::GR:
            return "gr";
        case Game::Country::HU:
            return "hu";
        case Game::Country::IL:
            return "il";
        case Game::Country::IT:
            return "it";
        case Game::Country::JP:
            return "jp";
        case Game::Country::KW:
            return "kw";
        case Game::Country::WOR:
            return "wor";
        case Game::Country::MOR:
            return "mor";
        case Game::Country::NO:
            return "no";
        case Game::Country::NZ:
            return "nz";
        case Game::Country::OCE:
            return "oce";
        case Game::Country::NL:
            return "nl";
        case Game::Country::PE:
            return "pe";
        case Game::Country::PL:
            return "pl";
        case Game::Country::PT:
            return "pt";
        case Game::Country::CZ:
            return "cz";
        case Game::Country::UK:
            return "uk";
        case Game::Country::RU:
            return "ru";
        case Game::Country::SS:
            return "ss";
        case Game::Country::SK:
            return "sk";
        case Game::Country::SE:
            return "se";
        case Game::Country::TW:
            return "tw";
        case Game::Country::TR:
            return "tr";
        case Game::Country::US:
            return "us";
        case Game::Country::ALL:
            return "all";
        default:
            return "unk";
    }
}

Game::Country Api::toCountry(const std::string &country) {
    if (country == "de") { return Game::Country::DE; }
    else if (country == "asi") { return Game::Country::ASI; }
    else if (country == "au") { return Game::Country::AU; }
    else if (country == "br") { return Game::Country::BR; }
    else if (country == "bg") { return Game::Country::BG; }
    else if (country == "ca") { return Game::Country::CA; }
    else if (country == "cl") { return Game::Country::CL; }
    else if (country == "cn") { return Game::Country::CN; }
    else if (country == "ame") { return Game::Country::AME; }
    else if (country == "kr") { return Game::Country::KR; }
    else if (country == "cus") { return Game::Country::CUS; }
    else if (country == "dk") { return Game::Country::DK; }
    else if (country == "sp") { return Game::Country::SP; }
    else if (country == "eu") { return Game::Country::EU; }
    else if (country == "fi") { return Game::Country::FI; }
    else if (country == "fr") { return Game::Country::FR; }
    else if (country == "gr") { return Game::Country::GR; }
    else if (country == "hu") { return Game::Country::HU; }
    else if (country == "il") { return Game::Country::IL; }
    else if (country == "it") { return Game::Country::IL; }
    else if (country == "jp") { return Game::Country::JP; }
    else if (country == "kw") { return Game::Country::KW; }
    else if (country == "wor") { return Game::Country::WOR; }
    else if (country == "mor") { return Game::Country::MOR; }
    else if (country == "no") { return Game::Country::NO; }
    else if (country == "nz") { return Game::Country::NZ; }
    else if (country == "oce") { return Game::Country::OCE; }
    else if (country == "nl") { return Game::Country::NL; }
    else if (country == "pe") { return Game::Country::PE; }
    else if (country == "pl") { return Game::Country::PL; }
    else if (country == "pt") { return Game::Country::PT; }
    else if (country == "cz") { return Game::Country::CZ; }
    else if (country == "uk") { return Game::Country::UK; }
    else if (country == "ru") { return Game::Country::RU; }
    else if (country == "ss") { return Game::Country::SS; }
    else if (country == "sk") { return Game::Country::SK; }
    else if (country == "se") { return Game::Country::SE; }
    else if (country == "tw") { return Game::Country::TW; }
    else if (country == "tr") { return Game::Country::TR; }
    else if (country == "us") { return Game::Country::US; }
    else if (country == "all") { return Game::Country::ALL; }
    else return Game::Country::UNK;
}

Game::Language Api::toLanguage(const std::string &language) {
    if (language == "en") { return Game::Language::EN; }
    else if (language == "fr") { return Game::Language::FR; }
    else if (language == "es") { return Game::Language::ES; }
    else if (language == "pt") { return Game::Language::PT; }
    else if (language == "all") { return Game::Language::ALL; }
    else return Game::Language::UNK;
}

bool Api::sortByName(const std::string &g1, const std::string &g2) {
#ifdef _MSC_VER
    return _stricmp(g1.c_str(), g2.c_str()) < 0;
#else
    return strcasecmp(g1.c_str(), g2.c_str()) < 0;
#endif
}

bool Api::sortGameByName(const Game &g1, const Game &g2) {
    const std::string lhs = g1.getName().text;
    const std::string rhs = g2.getName().text;
    const auto result = mismatch(
            lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto &lhs, const auto &rhs) {
                return tolower(lhs) == tolower(rhs);
            });
    return result.second != rhs.cend() &&
           (result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
}

bool Api::sortGameByPath(const Game &g1, const Game &g2) {
    const std::string lhs = g1.path;
    const std::string rhs = g2.path;
    const auto result = mismatch(
            lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const auto &lhs, const auto &rhs) {
                return tolower(lhs) == tolower(rhs);
            });
    return result.second != rhs.cend() &&
           (result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
}

int Api::parseInt(const std::string &str, int defValue) {
    char *end = nullptr;
    long i = strtol(str.c_str(), &end, 10);
    if (end != nullptr) {
        return (int) i;
    }
    return defValue;
}

long Api::parseLong(const std::string &str, long defValue) {
    char *end = nullptr;
    long i = strtol(str.c_str(), &end, 10);
    if (end != nullptr) {
        return (int) i;
    }
    return defValue;
}

bool Api::parseBool(const std::string &str, bool defValue) {
    if (str.empty() || str != "true" || str != "false"
        || str != "0" || str != "1") {
        return defValue;
    }

    return str == "true" || str == "1";
}

#ifdef __WINDOWS__
void Api::printc(int color, const char* format, ...) {
#else

void Api::printc(const char *color, const char *format, ...) {
#endif

    char buffer[1024];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buffer, 1024, format, arg);
    va_end(arg);

#ifdef __WINDOWS__
    WORD consoleAttr = FOREGROUND_INTENSITY | FOREGROUND_RED
                       | FOREGROUND_GREEN | FOREGROUND_BLUE;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(h, &csbi)) {
        consoleAttr = csbi.wAttributes;
    }
    SetConsoleTextAttribute(h, color);
    printf("%s", buffer);
    SetConsoleTextAttribute(h, consoleAttr);
#else
    printf("%s%s" "\033[0m", color, buffer);
#endif
}

void Api::printe(int code, int delay) {
    if (code == 429) {
        printc(COLOR_O, "NOK: maximum requests per minute reached... retrying in %i seconds\n", delay);
    } else if (code == 28) {
        printc(COLOR_O, "NOK: timeout reached... retrying in %i seconds\n", delay);
    }
}
