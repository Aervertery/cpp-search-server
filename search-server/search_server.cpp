#include "search_server.h"

SearchServer::SearchServer(const std::string& text) : SearchServer(SplitIntoWords(text)) {
    }

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings) {
    if (document_id < 0 || documents_.count(document_id) != 0 || !IsValidWord(document)) {
        throw std::invalid_argument("Invalid document data"s);
    }
    const std::vector<std::string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        documents_freqs_[word][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    documents_ids_.push_back(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus status_, int rating) {
        return status_ == status; });
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query,
    int document_id) const {
    const QueryContent query = ParseQuery(raw_query);
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words_) {
        if (documents_freqs_.count(word) != 0) {
            if (documents_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
    }
    for (const std::string& word : query.minus_words_) {
        if (documents_freqs_.count(word) != 0) {
            if (documents_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

int SearchServer::GetDocumentId(int index) const {
    if (index < 0 || index >= documents_.size()) {
        throw std::out_of_range("Invalid document id"s);
    }
    return documents_ids_[index];
}

double SearchServer::ComputeIdf(const std::string& word) const {
    return log((GetDocumentCount() * 1.0) / documents_freqs_.at(word).size());
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings) {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}