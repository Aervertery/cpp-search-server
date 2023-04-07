#pragma once
#include <set>
#include <algorithm>
#include <vector>
#include <stdexcept>

using std::literals::string_literals::operator""s;

std::vector<std::string> SplitIntoWords(const std::string& text);

extern std::set<std::string> stop_words_processing;

template <typename StringContainer>
std::set<std::string> SplitInputStringsContainerIntoStrings(const StringContainer& input_strings) {
    std::set<std::string> result;
    for (const std::string& element : input_strings) {
        for (const std::string& word : SplitIntoWords(element)) {
            result.insert(word);
        }
    }
    stop_words_processing = result;
    return result;
}
    
struct QueryContent {
    std::set<std::string> plus_words_;
    std::set<std::string> minus_words_;
};

struct QueryWordContent {
    std::string word;
    bool IsMinus;
    bool IsStop;
};

bool IsValidWord(const std::string& word);

QueryWordContent IsMinusWord(const std::string& word);
    
QueryContent ParseQuery(const std::string& text);  

bool IsStopWord(const std::string& word);

std::vector<std::string> SplitIntoWordsNoStop(const std::string& text);

