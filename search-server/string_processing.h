#pragma once
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <set>

std::vector<std::string> SplitIntoWords(const std::string& text);

std::vector<std::string> SplitIntoWords(std::string_view text);

std::vector<std::string_view> SplitIntoWordsView(std::string_view str);