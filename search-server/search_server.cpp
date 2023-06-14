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
        documents_[document_id].word_frequencies[word] += inv_word_count;
    }
    documents_[document_id].rating = ComputeAverageRating(ratings);
    documents_[document_id].status = status;
    documents_ids_.emplace(document_id);
    const std::set<std::string> words_ = std::set(words.begin(), words.end());
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
    //LOG_DURATION_STREAM(__func__, std::cout); {
        //std::cout << raw_query << std::endl;
        const QueryContent query = ParseQuery(raw_query);
        std::vector<std::string> matched_words;
        for (const std::string& word : query.minus_words_) {
            if (documents_freqs_.count(word) != 0) {
                if (documents_freqs_.at(word).count(document_id)) {
                    return { matched_words, documents_.at(document_id).status };
                }
            }
        }
        for (const std::string& word : query.plus_words_) {
            if (documents_freqs_.count(word) != 0) {
                if (documents_freqs_.at(word).count(document_id)) {
                    matched_words.push_back(word);
                }
            }
        }
        return { matched_words, documents_.at(document_id).status };
    //}
}

bool SearchServer::IsValidWord(const std::string& word) const {
    // A valid word must not contain special characters
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

SearchServer::QueryWordContent SearchServer::IsMinusWord(const std::string& word) const {
    if (!IsValidWord(word)) {
        throw std::invalid_argument("This word is invalid"s);
    }
    if (word[0] == '-') {
        const std::string new_word = word.substr(1);
        if (new_word.empty() || new_word[0] == '-') {
            throw std::invalid_argument("Invalid word"s);
        }
        return { new_word, true, IsStopWord(new_word) };
    }
    return { word, false, IsStopWord(word) };
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

SearchServer::QueryContent SearchServer::ParseQuery(const std::string& text, bool if_par) const {
    QueryContent query;
    for (std::string& word : SplitIntoWords(text)) {
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
        RemoveDuplicates(query.plus_words_);
        RemoveDuplicates(query.minus_words_);
    }
    return query;
}

std::set<int>::const_iterator SearchServer::begin() const {
    return SearchServer::documents_ids_.begin();
}

std::set<int>::const_iterator SearchServer::end() const {
    return SearchServer::documents_ids_.end();
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

const std::map<std::string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    static const std::map<std::string, double> empty_map;
    auto it = documents_.lower_bound(document_id);
    if (it != documents_.end()) {
        return documents_.at(document_id).word_frequencies;
    }
    return empty_map;
}

void SearchServer::RemoveDocument(int document_id) {
    documents_ids_.erase(document_id);
    auto it = documents_.find(document_id);
    for (auto& [word, freq] : it->second.word_frequencies) {
        documents_freqs_[word].erase(documents_freqs_[word].find(it->first));
    }
    documents_.erase(document_id);
    return ;
}

void AddDocument(SearchServer& search_server, int document_id, const std::string& raw_query, DocumentStatus status,
    const std::vector<int>& ratings) {
    search_server.AddDocument(document_id, raw_query, status, ratings);
}

void SearchServer::RemoveDuplicates(std::vector<std::string>& words) const {
    std::sort(words.begin(), words.end());
    words.erase(std::unique(words.begin(), words.end()), words.end());
}

