#include "document.h"

using std::literals::string_literals::operator""s;

Document::Document() :
        id(0),
        relevance(0.0),
        rating(0) {
        }

Document::Document(int id_, double relevance_, int rating_) :
        id(id_),
        relevance(relevance_),
        rating(rating_) {
        }

std::ostream& operator<<(std::ostream& os, const Document& document) {
    os << "{ document_id = "s << document.id <<
        ", relevance = "s << document.relevance <<
        ", rating = "s << document.rating << " }"s;
    return os;
}