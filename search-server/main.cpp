#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

const double ALLOWABLE_ERROR = 1e-6;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}
int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}
vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}
struct Document {
    Document() :
        id(0),
        relevance(0.0),
        rating(0)
    {
    }

    Document(int id_, double relevance_, int rating_) :
        id(id_),
        relevance(relevance_),
        rating(rating_)
    {
    }
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
    SearchServer(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            if (!IsValidWord(text)) {
                throw invalid_argument("This stop-word contains invalid characters"s);
            }
            stop_words_.insert(word);
        }
    }

    template <typename T>
    SearchServer(const T& text) {
        for (const string& element : text) {
            for (const string& word : SplitIntoWords(element)) {
                if (!IsValidWord(element)) {
                    throw invalid_argument("This stop-word contains invalid characters"s);
                }
                stop_words_.insert(word);
            }
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
        const vector<int>& ratings) {
        if (document_id < 0 || documents_.count(document_id) != 0 || !IsValidWord(document)) {
            throw invalid_argument("Invalid document data"s);
        }
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            documents_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus status_, int rating) {
            return status_ == status; });
    }
    size_t GetDocumentCount() const {
        return documents_.size();
    }

    template <typename Predicate>
    vector<Document> FindTopDocuments(const string& raw_query,
        Predicate predicate) const {
        const QueryContent query = ParseQuery(raw_query);
        vector<Document> matched_documents = FindAllDocuments(query, predicate);

        sort(matched_documents.begin(), matched_documents.end(),
            [predicate](const Document& lhs, const Document& rhs) {
                if (abs(lhs.relevance - rhs.relevance) < ALLOWABLE_ERROR) {
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

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
        int document_id) const {
        const QueryContent query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words_) {
            if (documents_freqs_.count(word) != 0) {
                if (documents_freqs_.at(word).count(document_id)) {
                    matched_words.push_back(word);
                }
            }
        }
        for (const string& word : query.minus_words_) {
            if (documents_freqs_.count(word) != 0) {
                if (documents_freqs_.at(word).count(document_id)) {
                    matched_words.clear();
                    break;
                }
            }
        }
        return { matched_words, documents_.at(document_id).status };
    }

    int GetDocumentId(int index) const {
        if (index < 0 || index >= documents_.size()) {
            throw out_of_range("Invalid document id"s);
        }
        auto result = next(documents_.begin(), index);
        return result->first;
    }
private:
    struct QueryContent {
        set<string> plus_words_;
        set<string> minus_words_;
    };
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    struct QueryWordContent {
        string word;
        bool IsMinus;
    };
    map<string, map<int, double>> documents_freqs_; //словарь слово -> (словарь id документа -> Term frequency слова в этом документе)
    set<string> stop_words_;
    map<int, DocumentData> documents_;

    static bool IsValidWord(const string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
            });
    }

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    QueryWordContent IsMinusWord(const string& word) const {
        string new_word;
        if (!IsValidWord(word)) {
            throw invalid_argument("This word is invalid"s);
        }
        if (word[0] == '-') {
            new_word = word.substr(1);
            if (new_word.empty() || new_word[0] == '-') {
                throw invalid_argument("Invalid word"s);
            }
            return { new_word, true };
        }
        return { word, false };
    }

    QueryContent ParseQuery(const string& text) const {
        QueryContent query;
        for (string& word : SplitIntoWordsNoStop(text)) {
            QueryWordContent element = IsMinusWord(word);
            if (element.IsMinus) {
                query.minus_words_.insert(element.word);
            }
            else {
                query.plus_words_.insert(element.word);
            }
        }
        return query;
    }

    double ComputeIdf(const string& word) const {
        return log((GetDocumentCount() * 1.0) / documents_freqs_.at(word).size());
    }

    template <typename Predicate>
    vector<Document> FindAllDocuments(const QueryContent& query, Predicate predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words_) {
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
        for (const string& word : query.minus_words_) {
            if (documents_freqs_.count(word) != 0) {
                for (const auto [document_id, _] : documents_freqs_.at(word)) {
                    document_to_relevance.erase(document_id);
                }
            }
        }
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};