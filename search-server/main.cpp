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
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        ++document_count_;
        const vector<string> words = SplitIntoWordsNoStop(document);
        double term_frequency = 1.0 / words.size();
        for (const string& word : words) {
            documents_freqs_[word][document_id] += term_frequency;
        }

    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const QueryContent query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:

    struct QueryContent {
        set<string> plus_words_;
        set<string> minus_words_;
    };

    map<string, map<int, double>> documents_freqs_; //словарь слово -> (словарь id документа -> Term frequency слова в этом документе)

    set<string> stop_words_;

    int document_count_ = 0;

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

    QueryContent ParseQuery(const string& text) const {
        QueryContent query;
        string new_word;
        for (string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                new_word = word.substr(1);
                query.minus_words_.insert(new_word);
            }
            else {
                query.plus_words_.insert(word);
            }
        }
        return query;
    }

    double ComputeIdf (const string& word) const {
        return log((document_count_ * 1.0) / documents_freqs_.at(word).size());
    }

    vector<Document> FindAllDocuments(const QueryContent& query_words) const {
        vector<Document> matched_documents;
        map<int, double> id_relevance;
        for (const auto& word : query_words.plus_words_) {
            if (documents_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_frequency = ComputeIdf(word);
            for (const auto [document_id, term_frequency] : documents_freqs_.at(word)) {
                id_relevance[document_id] += inverse_document_frequency * term_frequency;
            }
        }
        for (const auto& word : query_words.minus_words_) {
            if (documents_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, term_frequency] : documents_freqs_.at(word)) {
                id_relevance.erase(document_id);
            }
        }
        for (const auto [document_id, relevance] : id_relevance) {
            matched_documents.push_back({ document_id, relevance });
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }
    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}