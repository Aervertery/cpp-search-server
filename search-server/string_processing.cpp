#include "string_processing.h"

using std::literals::string_literals::operator""s;

std::vector<std::string> SplitIntoWords(std::string_view text) {
    std::vector<std::string> words;
    std::string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

std::vector<std::string_view> SplitIntoWordsView(std::string_view str) {
    std::vector<std::string_view> result;
    str.remove_prefix(std::min(str.size(), str.find_first_not_of(" ")));
    const int64_t pos_end = str.npos;

    while (!str.empty()) {
        int64_t space = str.find(' ');
        result.push_back(space == pos_end ? str.substr(0) : str.substr(0, space));
        str.remove_prefix(std::min(str.size(), str.find_first_not_of(" ", space)));
    }

    return result;
}