#include "search_server.h"

SearchServer::SearchServer(const std::string& text) : SearchServer(SplitIntoWords(text)) {
}

SearchServer::SearchServer(const std::string_view& text) : SearchServer(SplitIntoWords(text)) {
}

void SearchServer::AddDocument(int document_id, std::string_view document_, DocumentStatus status,
    const std::vector<int>& ratings) {
    storage.emplace_back(document_);
    if (document_id < 0 || documents_.count(document_id) != 0 || !IsValidWord(storage.back())) {
        throw std::invalid_argument("Invalid document data"s);
    }
    const std::vector<std::string_view> words = SplitIntoWordsNoStop(storage.back());
    const double inv_word_count = 1.0 / words.size();
    for (const std::string_view& word : words) {
        documents_freqs_[word][document_id] += inv_word_count;
        documents_[document_id].word_frequencies[word] += inv_word_count;
    }
    documents_[document_id].rating = ComputeAverageRating(ratings);
    documents_[document_id].status = status;
    documents_ids_.emplace(document_id);
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(std::execution::seq, raw_query, [status](int document_id, DocumentStatus status_, int rating) {
        return status_ == status; });
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
}

MatchedDocument SearchServer::MatchDocument(std::string_view raw_query,
    int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

MatchedDocument SearchServer::MatchDocument(Sequenced, std::string_view raw_query,
    int document_id) const {
    const QueryContent query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;
    for (std::string_view word : query.minus_words_) {
        if (documents_freqs_.count(word) != 0) {
            if (documents_freqs_.at(word).count(document_id)) {
                return { matched_words, documents_.at(document_id).status };
            }
        }
    }
    for (std::string_view word : query.plus_words_) {
        if (documents_freqs_.count(word) != 0) {
            if (documents_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

MatchedDocument SearchServer::MatchDocument(Parallel, std::string_view raw_query,
    int document_id) const {
    std::vector<std::string_view> matched_words;
    const QueryContent query = ParseQuery(raw_query, true);
    if (std::any_of(std::execution::par,
        query.minus_words_.begin(), query.minus_words_.end(),
        [&](std::string_view word) {
            return documents_freqs_.count(word) != 0 && documents_freqs_.at(word).count(document_id);
        })) {
            return { matched_words, documents_.at(document_id).status };
    }
    matched_words.resize(query.plus_words_.size());
    auto it = std::copy_if(std::execution::par,
        query.plus_words_.begin(), query.plus_words_.end(),
        matched_words.begin(),
        [&](std::string_view word) {
            return documents_freqs_.count(word) != 0 && documents_freqs_.at(word).count(document_id);
        });
    std::sort(std::execution::par, matched_words.begin(), it);
    matched_words.erase(std::unique(std::execution::par, matched_words.begin(), it), matched_words.end());
    return { matched_words, documents_.at(document_id).status };
}

bool SearchServer::IsValidWord(std::string_view word) const {
    // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

SearchServer::QueryWordContent SearchServer::IsMinusWord(std::string_view word) const {
    if (!IsValidWord(word)) {
        throw std::invalid_argument("This word is invalid"s);
    }
    if (word[0] == '-') {
        const std::string_view new_word = word.substr(1);
        if (new_word.empty() || new_word[0] == '-') {
            throw std::invalid_argument("Invalid word"s);
        }
        return { new_word, true, IsStopWord(new_word) };
    }
    return { word, false, IsStopWord(word) };
}

bool SearchServer::IsStopWord(std::string_view word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string_view> words;
    for (const std::string_view& word : SplitIntoWordsView(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryContent SearchServer::ParseQuery(std::string_view text, bool if_par) const {
    QueryContent query;
    for (std::string_view word : SplitIntoWordsView(text)) {
        QueryWordContent element = IsMinusWord(word);
        if (!element.IsStop) {
            if (element.IsMinus) {
                query.minus_words_.push_back(element.word);
            }
            else {
                query.plus_words_.push_back(element.word);
            }
        }
    }
    if (!if_par) {
        RemoveDuplicatesWords(query.plus_words_);
        RemoveDuplicatesWords(query.minus_words_);
    }
    return query;
}

std::set<int>::const_iterator SearchServer::begin() const {
    return SearchServer::documents_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const {
    return SearchServer::documents_ids_.end();
}

double SearchServer::ComputeIdf(std::string_view word) const {
    return log((GetDocumentCount() * 1.0) / documents_freqs_.at(word).size());
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = std::accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string_view, double> empty_map;
    auto it = documents_.lower_bound(document_id);
    if (it != documents_.end()) {
        return documents_.at(document_id).word_frequencies;
    }
    return empty_map;
}

void SearchServer::RemoveDocument(int document_id) {
    if (documents_ids_.count(document_id) == 0) {
        return;
    }
    return RemoveDocument(std::execution::seq, document_id);
}

void SearchServer::RemoveDocument(Sequenced, int document_id) {
    documents_ids_.erase(document_id);
    auto it = documents_.find(document_id);
    for (auto& [word, freq] : it->second.word_frequencies) {
        documents_freqs_[word].erase(documents_freqs_[word].find(it->first));
    }
    documents_.erase(document_id);
    return;
}

void SearchServer::RemoveDocument(Parallel, int document_id) {
    if (documents_ids_.count(document_id) == 0) {
        return;
    }

    documents_ids_.erase(document_id);

    std::vector<const std::string*> words(documents_.at(document_id).word_frequencies.size());
    std::transform(std::execution::par,
        documents_[document_id].word_frequencies.begin(), documents_[document_id].word_frequencies.end(),
        words.begin(),
        [&](const auto& element) { static std::string tmp(element.first); return &tmp; });
    std::for_each(std::execution::par,
        words.begin(), words.end(),
        [&](const std::string* word) { auto it = documents_.find(document_id);
    documents_freqs_[*word].erase(documents_freqs_[*word].find(it->first)); });


    documents_.erase(document_id);
    return;
}

void AddDocument(SearchServer& search_server, int document_id, std::string_view raw_query, DocumentStatus status,
    const std::vector<int>& ratings) {
    search_server.AddDocument(document_id, raw_query, status, ratings);
}

void SearchServer::RemoveDuplicatesWords(std::vector<std::string_view>& words) const {
    std::sort(words.begin(), words.end());
    words.erase(std::unique(words.begin(), words.end()), words.end());
}

