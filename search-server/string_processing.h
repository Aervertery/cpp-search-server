#pragma once
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <set>

std::vector<std::string> SplitIntoWords(const std::string& text);

bool IsValidWord(const std::string& word);

struct QueryContent {
    std::set<std::string> plus_words_;
    std::set<std::string> minus_words_;
};

struct QueryWordContent {
    std::string word;
    bool IsMinus;
    bool IsStop;
};

bool IsStopWord(const std::string& word, const std::set<std::string>& stop_words);

QueryWordContent IsMinusWord(const std::string& word, const std::set<std::string>& stop_words);

std::vector<std::string> SplitIntoWordsNoStop(const std::string& text, const std::set<std::string>& stop_words);