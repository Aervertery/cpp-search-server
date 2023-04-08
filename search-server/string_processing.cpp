#include "string_processing.h"

using std::literals::string_literals::operator""s;

std::set<std::string> stop_words_processing;

std::vector<std::string> SplitIntoWords(const std::string& text) {
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

bool IsValidWord(const std::string& word) {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

QueryWordContent IsMinusWord(const std::string& word) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("This word is invalid"s);
        }
        if (word[0] == '-') {
            const std::string new_word = word.substr(1);
            if (new_word.empty() || new_word[0] == '-') {
                throw std::invalid_argument("Invalid word"s);
            }
            return { new_word, true, IsStopWord(new_word) };
        }
        return { word, false, IsStopWord(word) };
    }

QueryContent ParseQuery(const std::string& text) {
    QueryContent query;
    for (std::string& word : SplitIntoWords(text)) {
        QueryWordContent element = IsMinusWord(word);
        if (!element.IsStop) {
            if (element.IsMinus) {
                query.minus_words_.insert(element.word);
            }
            else {
                query.plus_words_.insert(element.word);
            }
        }
    }
    return query;
}  

bool IsStopWord(const std::string& word) {
    return stop_words_processing.count(word) > 0;
}

std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}