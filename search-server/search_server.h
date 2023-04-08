#pragma once
#include <cmath>
#include <map>
#include <cmath>
#include "document.h"
#include "string_processing.h"
#include "read_input_functions.h"

using std::literals::string_literals::operator""s;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

const double ALLOWABLE_ERROR = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    SearchServer(const StringContainer& text);

    SearchServer(const std::string& text);

    void AddDocument(int document_id, const std::string& document, DocumentStatus status,
        const std::vector<int>& ratings);

    std::vector<Document> FindTopDocuments(const std::string& raw_query) const;

    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status) const;
    
    size_t GetDocumentCount() const;

    template <typename Predicate>
    std::vector<Document> FindTopDocuments(const std::string& raw_query,
        Predicate predicate) const;

    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query,
        int document_id) const;

    int GetDocumentId(int index) const;
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };


    struct QueryContent {
        std::set<std::string> plus_words_;
        std::set<std::string> minus_words_;
    };

    struct QueryWordContent {
        std::string word;
        bool IsMinus;
        bool IsStop;
    };

    std::map<std::string, std::map<int, double>> documents_freqs_; //словарь слово -> (словарь id документа -> Term frequency слова в этом документе)
    std::set<std::string> stop_words_;
    std::map<int, DocumentData> documents_;
    std::vector<int> documents_ids_;

    template <typename StringContainer>
    std::set<std::string> SplitInputStringsContainerIntoStrings(const StringContainer& input_strings);

    QueryWordContent IsMinusWord(const std::string& word) const;

    QueryContent ParseQuery(const std::string& text) const;

    bool IsStopWord(const std::string& word) const;

    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;

    double ComputeIdf(const std::string& word) const;  

    static int ComputeAverageRating(const std::vector<int>& ratings);

    template <typename Predicate>
    std::vector<Document> FindAllDocuments(const QueryContent& query, Predicate predicate) const;
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& text) : stop_words_(SplitInputStringsContainerIntoStrings(text)) {
    for (const std::string& word : stop_words_) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("This stop-word contains invalid characters"s);
        }
    }
}

template <typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query,
    Predicate predicate) const {
    const QueryContent query = ParseQuery(raw_query);
    std::vector<Document> matched_documents = FindAllDocuments(query, predicate);
    sort(matched_documents.begin(), matched_documents.end(),
        [predicate](const Document& lhs, const Document& rhs) {
            if (std::abs(lhs.relevance - rhs.relevance) < ALLOWABLE_ERROR) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const QueryContent& query, Predicate predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string& word : query.plus_words_) {
        if (documents_freqs_.count(word) != 0) {
            const double inverse_document_frequency = ComputeIdf(word);
            for (const auto [document_id, term_freq] : documents_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_frequency;
                }
            }
        }
    }
    for (const std::string& word : query.minus_words_) {
        if (documents_freqs_.count(word) != 0) {
            for (const auto [document_id, _] : documents_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
    }
    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            { document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

template <typename StringContainer>
std::set<std::string> SearchServer::SplitInputStringsContainerIntoStrings(const StringContainer& input_strings) {
    std::set<std::string> result;
    for (const std::string& element : input_strings) {
        for (const std::string& word : SplitIntoWords(element)) {
            result.insert(word);
        }
    }
    return result;
}