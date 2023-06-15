#include "test_example_functions.h"

/*#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint) {
    if (!value) {
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

#define RUN_TEST(func) RunTestImpl((func), #func)

// “ест провер€ет, что поискова€ система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server(""s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
            "Stop words must be excluded from documents"s);
    }
}

//“ест провер€ет добавление документа и его нахождение по запросу, который содержит
//слова из этого документа
void TestAddingDocument() {
    const int doc_id = 42;
    const std::string content = "big purple cat"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found = server.FindTopDocuments("purple"s);
        ASSERT_EQUAL(found.size(), 1);
        const auto found1 = server.FindTopDocuments("dog"s);
        ASSERT_EQUAL(found1.size(), 0);
        const auto found2 = server.FindTopDocuments(""s);
        ASSERT_EQUAL(found2.size(), 0);
        const auto found3 = server.FindTopDocuments(" "s);
        ASSERT_EQUAL(found3.size(), 0);
    }
}

//“ест провер€ет исключение из результатов поиска документов, содержащих минус-слова
void TestMinusWords() {
    const int doc_id = 42;
    const std::string content = "big purple cat"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small purple dog"s;
    const std::vector<int> ratings2 = { 1, 2, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple -dog"s);
        ASSERT_EQUAL(found.size(), 1);
        const auto found1 = server.FindTopDocuments("dog"s);
        ASSERT_EQUAL(found1.size(), 1);
        const auto found2 = server.FindTopDocuments("purple -cat -small"s);
        ASSERT_EQUAL(found2.size(), 0);
    }
}

//“ест провер€ет соответствие документов поисковому запросу
void TestDocumentMatching() {
    const int doc_id = 42;
    const std::string content = "big purple cat"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small purple dog"s;
    const std::vector<int> ratings2 = { 1, 2, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found0 = server.MatchDocument("purple -cat"s, doc_id);
        const auto found01 = std::get<0>(found0);
        ASSERT_EQUAL(found01.size(), 0);
        const auto found1 = server.MatchDocument("dog"s, doc_id2);
        const auto found11 = std::get<0>(found1);
        ASSERT_EQUAL(found11[0], "dog"s);
        const auto found2 = server.MatchDocument("purple big"s, doc_id2);
        const auto found21 = std::get<0>(found2);
        ASSERT_EQUAL(found21[0], "purple"s);
        const auto found3 = server.MatchDocument("purple big"s, doc_id);
        const auto found31 = std::get<0>(found3);
        ASSERT_EQUAL(found31.size(), 2);
    }
}

//“ест провер€ет сортировку найденных документов по убыванию релевантности
void TestDocumentSorting() {
    const int doc_id = 42;
    const std::string content = "big purple cat"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small purple dog"s;
    const std::vector<int> ratings2 = { 1, 2, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple dog"s);
        ASSERT(found[0].relevance > found[1].relevance);
    }
}

//“ест провер€ет вычисление рейтинга документа
void TestComputeDocumentRating() {
    const int doc_id = 42;
    const std::string content = "big purple cat"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple dog"s);
        ASSERT_EQUAL(found[0].rating, 1);
        ASSERT_EQUAL(found[1].rating, 2);
    }
}

//“ест провер€ет поиск документов по фильтру-предикату, задаваемому пользователем
void TestDocumentFilterUsingPredicate() {
    const int doc_id = 42;
    const std::string content = "big purple cat"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple dog"s, [](int document_id, DocumentStatus status, int rating)
            { return document_id % 2 == 0; });
        ASSERT_EQUAL(found[0].id, 42);
        ASSERT_EQUAL(found.size(), 1);
        const auto found0 = server.FindTopDocuments("big purple dog"s, [](int document_id, DocumentStatus status, int rating)
            { return rating > 0; });
        ASSERT_EQUAL(found0[0].id, 42);
        ASSERT_EQUAL(found0[1].id, 43);
        ASSERT_EQUAL(found0.size(), 2);
    }
}

//“ест провер€ет поиск документов по заданному статусу
void TestDocumentSearchWithCertainStatus() {
    const int doc_id = 42;
    const std::string content = "big purple cat"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    const int doc_id3 = 44;
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::BANNED, ratings2);
        const auto found = server.FindTopDocuments("purple dog"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(found.size(), 1);
        const auto found0 = server.FindTopDocuments("purple cat"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL(found0.size(), 1);
        AddDocument(server, doc_id3, content2, DocumentStatus::BANNED, ratings2);
        const auto found1 = server.FindTopDocuments("purple dog"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(found1.size(), 2);
        const auto found2 = server.FindTopDocuments("purple dog"s, DocumentStatus::IRRELEVANT);
        const auto found3 = server.FindTopDocuments("purple dog"s, DocumentStatus::REMOVED);
        ASSERT_EQUAL(found2.size(), 0);
        ASSERT_EQUAL(found3.size(), 0);
    }
}

//“ест провер€ет корректное вычисление релевантности найденных документов
void TestComputeDocumentRelevance() {
    const int doc_id = 42;
    const std::string content = "purple cat purple eyes"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small cowardly purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple dog");
        ASSERT_EQUAL(found[1].relevance, 0.0);
        ASSERT_EQUAL(found[0].relevance, log(2) * 0.25);
    }
}

//“ест провер€ет корректное разделение результатов поиска на страницы
void TestPaginate() {
    const int doc_id = 42;
    const std::string content = "purple cat purple eyes"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small cowardly purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server("in the"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found1 = server.FindTopDocuments("purple dog");
        const auto pages1 = Paginate(found1, 1);
        ASSERT_EQUAL(pages1.size(), 2);
        AddDocument(server, 44, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found2 = server.FindTopDocuments("purple dog");
        const auto pages2 = Paginate(found2, 2);
        ASSERT_EQUAL(pages2.size(), 2);
        AddDocument(server, 45, content2, DocumentStatus::ACTUAL, ratings2);
        AddDocument(server, 46, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found3 = server.FindTopDocuments("purple dog");
        const auto pages3 = Paginate(found3, 2);
        ASSERT_EQUAL(pages3.size(), 3);
    }
}

//“ест провер€ет корректное удаление документа из базы
void TestRemoveDocument() {
    const int doc_id = 42;
    const std::string content = "purple cat purple eyes"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small cowardly purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    const int doc_id3 = 44;
    const std::string content3 = "big brave kind panda and orange cat"s;
    const std::vector<int> ratings3 = { 10, 10, 10 };
    {
        SearchServer server("in the and"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        AddDocument(server, doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found1 = server.FindTopDocuments("cat orange eyes");
        ASSERT_EQUAL(found1.size(), 2);
        server.RemoveDocument(42);
        const auto found2 = server.FindTopDocuments("cat orange eyes");
        ASSERT_EQUAL(found2.size(), 1);
    }
}

//“ест провер€ет корректное получение частоты слов конкретного документа
void TestGetWordFrequencies() {
    const int doc_id = 42;
    const std::string content = "purple cat purple eyes"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small cowardly purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server("in the and"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found1 = server.GetWordFrequencies(42);
        ASSERT_EQUAL(found1.size(), 3);
        ASSERT_EQUAL(found1.at("purple"s), 2.0 / 4.0);
        const auto found2 = server.GetWordFrequencies(43);
        ASSERT_EQUAL(found2.size(), 4);
        ASSERT_EQUAL(found2.at("dog"s), 1.0 / 4.0);
        const auto found3 = server.GetWordFrequencies(44);
        ASSERT(found3.empty());
    }
}

//“ест провер€ет корректное удаление документов-дубликатов из базы
void TestRemoveDuplicates() {
    const int doc_id = 42;
    const std::string content = "purple cat purple eyes"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small cowardly purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    const int doc_id3 = 44;
    const std::string content3 = "small cowardly purple dog"s;
    const std::vector<int> ratings3 = { 1, -1, 3 };
    {
        SearchServer server("in the and"s);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found1 = server.FindTopDocuments("purple dog"s);
        ASSERT_EQUAL(found1.size(), 2);
        RemoveDuplicates(server);
        const auto found2 = server.FindTopDocuments("purple dog"s);
        ASSERT_EQUAL(found2.size(), 2);
        AddDocument(server, doc_id3, content3, DocumentStatus::ACTUAL, ratings3);
        const auto found3 = server.FindTopDocuments("purple dog"s);
        ASSERT_EQUAL(found3.size(), 3);
        RemoveDuplicates(server);
        const auto found4 = server.FindTopDocuments("purple dog"s);
        ASSERT_EQUAL(found4.size(), 2);
    }
}

//“ест провер€ет корректную работу очереди запросов
void TestRequestQueue() {
    const int doc_id = 42;
    const std::string content = "purple cat purple eyes"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const std::string content2 = "small cowardly purple dog"s;
    const std::vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server("in the and"s);
        RequestQueue queue(server);
        for (int i = 0; i < 1439; ++i) {
            queue.AddFindRequest("empty request"s);
        }
        ASSERT_EQUAL(queue.GetNoResultRequests(), 1440);
        AddDocument(server, doc_id, content, DocumentStatus::ACTUAL, ratings);
        AddDocument(server, doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        queue.AddFindRequest("black cat white tail"s);
        queue.AddFindRequest("purple dog badge courage");
        ASSERT_EQUAL(queue.GetNoResultRequests(), 1438);
    }
}

// ‘ункци€ TestSearchServer €вл€етс€ точкой входа дл€ запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestAddingDocument);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestDocumentMatching);
    RUN_TEST(TestDocumentSorting);
    RUN_TEST(TestComputeDocumentRating);
    RUN_TEST(TestDocumentFilterUsingPredicate);
    RUN_TEST(TestDocumentSearchWithCertainStatus);
    RUN_TEST(TestComputeDocumentRelevance);
    RUN_TEST(TestPaginate);
    RUN_TEST(TestRemoveDocument);
    RUN_TEST(TestGetWordFrequencies);
    RUN_TEST(TestRemoveDuplicates);
    RUN_TEST(TestRequestQueue);
}*/