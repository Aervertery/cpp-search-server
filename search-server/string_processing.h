#pragma once
#include <set>
#include <algorithm>
#include <vector>
#include <stdexcept>

std::vector<std::string> SplitIntoWords(const std::string& text);

bool IsValidWord(const std::string& word);

