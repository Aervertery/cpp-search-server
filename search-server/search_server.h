#pragma once
#include <cmath>
#include <deque>
#include <map>
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

    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    std::vector<Document> FindTopDocuments(std::string_view raw_query, DocumentStatus status) const;

    template <typename ExecutionPolicy, typename Predicate>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query,
        Predicate predicate) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query, DocumentStatus status) const;

    size_t GetDocumentCount() const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query,
        int document_id) const;

    template <typename ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, std::string_view raw_query,
        int document_id) const;

    std::set<int>::const_iterator begin() const;

    std::set<int>::const_iterator end() const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    void RemoveDocument(int document_id);

    template <typename ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id);
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
    std::vector<Document> FindAllDocuments(const QueryContent& query, Predicate predicate) const;

    template <typename ExecutionPolicy, typename Predicate>
    std::vector<Document> FindAllDocuments(ExecutionPolicy&& policy, const QueryContent& query, Predicate predicate) const;

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
    //LOG_DURATION_STREAM(__func__, std::cerr); {
        //std::cout << raw_query << std::endl;
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
    //}
}

template <typename ExecutionPolicy, typename Predicate>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query,
    Predicate predicate) const {
    if constexpr (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        return FindTopDocuments(raw_query, predicate);
    }
    else {
        //LOG_DURATION_STREAM(__func__, std::cerr); {
        //std::cout << raw_query << std::endl;
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
        //}
    }
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query) const {
    if constexpr (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }
    else {
        return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
    }
}

template <typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query, DocumentStatus status) const {
    if constexpr (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus status_, int rating) {
            return status_ == status; });
    }
    else {
        return FindTopDocuments(policy, raw_query, [status](int document_id, DocumentStatus status_, int rating) {
            return status_ == status; });
    }
}

template <typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(const QueryContent& query, Predicate predicate) const {
    //LOG_DURATION("FindAll"s);
    std::map<int, double> document_to_relevance;
    //{LOG_DURATION("Plus"s); 
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
//}
    //{LOG_DURATION("Minus"s); 
    for (std::string_view word : query.minus_words_) {
        if (documents_freqs_.count(word) != 0) {
            for (const auto [document_id, _] : documents_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
    }
//}
    std::vector<Document> matched_documents;
    //{LOG_DURATION("ToMatch"s);
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            { document_id, relevance, documents_.at(document_id).rating });
    //}
    }
    return matched_documents;
}

template <typename ExecutionPolicy, typename Predicate>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy, const QueryContent& query, Predicate predicate) const {
    if constexpr (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        return FindAllDocuments(query, predicate);
    }
    else {
        //LOG_DURATION("FindAllPar"s);
        std::map<int, double> document_to_relevance;
        ConcurrentMap<int, double> test(1000);
        //{LOG_DURATION("PlusPar"s);
        std::for_each(std::execution::par, query.plus_words_.begin(), query.plus_words_.end(), [&](std::string_view word) {
            if (documents_freqs_.count(word) != 0) {
                const double inverse_document_frequency = ComputeIdf(word);
                std::for_each(std::execution::par, documents_freqs_.at(word).begin(), documents_freqs_.at(word).end(), [&](const std::pair<int, double>& element) {
                    const auto& document_data = documents_.at(element.first);
                    if (predicate(element.first, document_data.status, document_data.rating)) {
                        test[element.first].ref_to_value += element.second * inverse_document_frequency;
                    }
                    });
            }
            });
        //}

        //{LOG_DURATION("MinusPar"s); 
        for (std::string_view word : query.minus_words_) {
            if (documents_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : documents_freqs_.at(word)) {
                    test.erase(document_id);
                }
            }
        }
        //}
        std::vector<Document> matched_documents;
        //{LOG_DURATION("toMatchPar"s);
        auto tmp = test.BuildOrdinaryMap();
        matched_documents.resize(tmp.size());
        for (const auto [document_id, relevance] : tmp) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        //}
        return matched_documents;
    }
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

template <typename ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, std::string_view raw_query,
    int document_id) const {
    //LOG_DURATION_STREAM(__func__, std::cout); {
        //std::cout << raw_query << std::endl;
    std::vector<std::string_view> matched_words;
    if constexpr (std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        return MatchDocument(raw_query, document_id);
    }
    else {
        const QueryContent query = ParseQuery(raw_query, true);
        if (std::any_of(policy,
            query.minus_words_.begin(), query.minus_words_.end(),
            [&](std::string_view word) {
                return documents_freqs_.count(word) != 0 && documents_freqs_.at(word).count(document_id);
            })) {
            return { matched_words, documents_.at(document_id).status };
        }
        matched_words.resize(query.plus_words_.size());
        auto it = std::copy_if(policy,
            query.plus_words_.begin(), query.plus_words_.end(),
            matched_words.begin(),
            [&](std::string_view word) {
                return documents_freqs_.count(word) != 0 && documents_freqs_.at(word).count(document_id);
            });
        std::sort(policy, matched_words.begin(), it);
        matched_words.erase(std::unique(policy, matched_words.begin(), it), matched_words.end());
    }
    return { matched_words, documents_.at(document_id).status };
    //}
}

template <typename ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id) {
    if (documents_ids_.count(document_id) == 0) {
        return;
    }
    documents_ids_.erase(document_id);
    if constexpr(std::is_same_v<std::decay_t<ExecutionPolicy>, std::execution::sequenced_policy>) {
        RemoveDocument(document_id);
    }
    else {
        std::vector<const std::string*> words(documents_.at(document_id).word_frequencies.size());
        std::transform(std::execution::par,
                       documents_[document_id].word_frequencies.begin(), documents_[document_id].word_frequencies.end(),
                       words.begin(),
                       [&](const auto& element) { return &(element.first); });
        std::for_each(std::execution::par,
                      words.begin(), words.end(),
                      [&](const std::string* word) { auto it = documents_.find(document_id);
                                                     documents_freqs_[*word].erase(documents_freqs_[*word].find(it->first)); });
    }
    documents_.erase(document_id);
    return;
}

void AddDocument(SearchServer& search_server, int document_id, std::string_view raw_query, DocumentStatus status,
    const std::vector<int>& ratings);