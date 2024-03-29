#pragma once
#include <cmath>
#include <deque>
#include <map>
#include <string>
#include <string_view>
#include <algorithm>
#include <execution>
#include <future>
#include "concurrent_map.h"
#include "document.h"
#include "read_input_functions.h"
#include "string_processing.h"
#include "log_duration.h"


using std::literals::string_literals::operator""s;

using MatchedDocument = std::tuple<std::vector<std::string_view>, DocumentStatus>;
using Parallel = std::execution::parallel_policy;
using Sequenced = std::execution::sequenced_policy;


const int MAX_RESULT_DOCUMENT_COUNT = 5;

const double ALLOWABLE_ERROR = 1e-6;

class SearchServer {
public:

    template <typename StringContainer>
    SearchServer(const StringContainer& text);

    SearchServer(const std::string& text);

    SearchServer(const std::string_view& text);

    void AddDocument(int document_id, std::string_view document_, DocumentStatus status,
        const std::vector<int>& ratings);

    template <typename Predicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
        Predicate predicate) const;

    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const;

    template <typename ExecutionPolicy, typename Predicate>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query,
        Predicate predicate) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const;

    size_t GetDocumentCount() const;

    MatchedDocument MatchDocument(std::string_view raw_query,
        int document_id) const;

    MatchedDocument MatchDocument(Sequenced, std::string_view raw_query,
        int document_id) const;

    MatchedDocument MatchDocument(Parallel, std::string_view raw_query,
        int document_id) const;

    std::set<int>::const_iterator begin() const;

    std::set<int>::const_iterator end() const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

    void RemoveDocument(Sequenced, int document_id);

    void RemoveDocument(Parallel, int document_id);
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
        std::map<std::string_view, double> word_frequencies; //словарь слово из документа -> частота появления этого слова в этом документе
    };

    std::map<std::string_view, std::map<int, double>> documents_freqs_; //словарь слово -> (словарь id документа -> Term frequency слова в этом документе)
    std::set<std::string, std::less<>> stop_words_; 
    std::map<int, DocumentData> documents_; //словарь номер документа -> информация о документе
    std::set<int> documents_ids_;
    std::deque<std::string> storage;

    struct QueryContent {
        std::vector<std::string_view> plus_words_;
        std::vector<std::string_view> minus_words_;
    };

    struct QueryWordContent {
        std::string_view word;
        bool IsMinus;
        bool IsStop;
    };

    template <typename StringContainer>
    std::set<std::string, std::less<>> SplitInputStringsContainerIntoStrings(const StringContainer& input_strings);

    bool IsValidWord(std::string_view word) const;

    bool IsStopWord(std::string_view word) const;

    QueryWordContent IsMinusWord(std::string_view word) const;

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string& text) const;

    QueryContent ParseQuery(std::string_view text, bool if_par = false) const;

    double ComputeIdf(std::string_view word) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    template <typename Predicate>
    std::vector<Document> FindAllDocuments(Sequenced, const QueryContent& query, Predicate predicate) const;

    template <typename Predicate>
    std::vector<Document> FindAllDocuments(Parallel, const QueryContent& query, Predicate predicate) const;

    void RemoveDuplicatesWords(std::vector<std::string_view>& words) const;
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
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
    Predicate predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, predicate);
}

template <typename ExecutionPolicy, typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query,
    Predicate predicate) const {
        const QueryContent query = ParseQuery(raw_query);
        std::vector<Document> matched_documents;
        matched_documents.reserve(storage.size());
        matched_documents = FindAllDocuments(policy, query, predicate);        
        sort(policy, matched_documents.begin(), matched_documents.end(),
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

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus status_, int rating) {
            return status_ == status; });
}

template < typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(Sequenced, const QueryContent& query, Predicate predicate) const {
    std::map<int, double> document_to_relevance;
    for (std::string_view word : query.plus_words_) {
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
    for (std::string_view word : query.minus_words_) {
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

template <typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(Parallel, const QueryContent& query, Predicate predicate) const {
        ConcurrentMap<int, double> doc_to_relev_concur(1000);
        std::for_each(std::execution::par, query.plus_words_.begin(), query.plus_words_.end(), [&](std::string_view word) {
            if (documents_freqs_.count(word) != 0) {
                const double inverse_document_frequency = ComputeIdf(word);
                std::for_each(std::execution::par, documents_freqs_.at(word).begin(), documents_freqs_.at(word).end(), [&](const std::pair<int, double>& element) {
                    const auto& document_data = documents_.at(element.first);
                    if (predicate(element.first, document_data.status, document_data.rating)) {
                        doc_to_relev_concur[element.first].ref_to_value += element.second * inverse_document_frequency;
                    }
                    });
            }
            });
        for (std::string_view word : query.minus_words_) {
            if (documents_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : documents_freqs_.at(word)) {
                    doc_to_relev_concur.erase(document_id);
                }
            }
        }
        std::vector<Document> matched_documents;
        std::map<int, double> document_to_relevance = doc_to_relev_concur.BuildOrdinaryMap();
        matched_documents.reserve(document_to_relevance.size());
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
}

template <typename StringContainer>
std::set<std::string, std::less<>> SearchServer::SplitInputStringsContainerIntoStrings(const StringContainer& input_strings) {
    std::set<std::string, std::less<>> result;
    for (const std::string& element : input_strings) {
        for (const std::string& word : SplitIntoWords(element)) {
            result.insert(word);
        }
    }
    return result;
}

void AddDocument(SearchServer& search_server, int document_id, std::string_view raw_query, DocumentStatus status,
    const std::vector<int>& ratings);