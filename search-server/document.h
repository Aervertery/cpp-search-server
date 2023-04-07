#pragma once
#include <iostream>

struct Document {
    Document();

    Document(int id_, double relevance_, int rating_);
    
    int id;
    double relevance;
    int rating;
};

std::ostream& operator<<(std::ostream& os, const Document& document);

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};