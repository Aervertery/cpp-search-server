// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

//Тест проверяет добавление документа и его нахождение по запросу, который содержит
//слова из этого документа
void TestAddingDocument() {
    const int doc_id = 42;
    const string content = "big purple cat"s;
    const vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
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

//Тест проверяет исключение из результатов поиска документов, содержащих минус-слова
void TestMinusWords() {
    const int doc_id = 42;
    const string content = "big purple cat"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const string content2 = "small purple dog"s;
    const vector<int> ratings2 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple -dog"s);
        ASSERT_EQUAL(found.size(), 1);
        const auto found1 = server.FindTopDocuments("dog"s);
        ASSERT_EQUAL(found1.size(), 1);
        const auto found2 = server.FindTopDocuments("purple -cat -small"s);
        ASSERT_EQUAL(found2.size(), 0);
    }
}

//Тест проверяет соответствие документов поисковому запросу
void TestDocumentMatching() {
    const int doc_id = 42;
    const string content = "big purple cat"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const string content2 = "small purple dog"s;
    const vector<int> ratings2 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found0 = server.MatchDocument("purple -cat"s, doc_id);
        const auto found01 = get<0>(found0);
        ASSERT_EQUAL(found01.size(), 0);
        const auto found1 = server.MatchDocument("dog"s, doc_id2);
        const auto found11 = get<0>(found1);
        ASSERT_EQUAL(found11[0], "dog"s);
        const auto found2 = server.MatchDocument("purple big"s, doc_id2);
        const auto found21 = get<0>(found2);
        ASSERT_EQUAL(found21[0], "purple"s);
        const auto found3 = server.MatchDocument("purple big"s, doc_id);
        const auto found31 = get<0>(found3);
        ASSERT_EQUAL(found31.size(), 2);
    }
}

//Тест проверяет сортировку найденных документов по убыванию релевантности
void TestDocumentSorting() {
    const int doc_id = 42;
    const string content = "big purple cat"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const string content2 = "small purple dog"s;
    const vector<int> ratings2 = { 1, 2, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple dog"s);
        ASSERT(found[0].relevance > found[1].relevance);
    }
}

//Тест проверяет вычисление рейтинга документа
void TestComputeDocumentRating() {
    const int doc_id = 42;
    const string content = "big purple cat"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const string content2 = "small purple dog"s;
    const vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple dog"s);
        ASSERT_EQUAL(found[0].rating, 1);
        ASSERT_EQUAL(found[1].rating, 2);
    }
}

//Тест проверяет поиск документов по фильтру-предикату, задаваемому пользователем
void TestDocumentFilterUsingPredicate() {
    const int doc_id = 42;
    const string content = "big purple cat"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const string content2 = "small purple dog"s;
    const vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
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

//Тест проверяет поиск документов по заданному статусу
void TestDocumentSearchWithCertainStatus() {
    const int doc_id = 42;
    const string content = "big purple cat"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const string content2 = "small purple dog"s;
    const vector<int> ratings2 = { 1, -1, 3 };
    const int doc_id3 = 44;
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::BANNED, ratings2);
        const auto found = server.FindTopDocuments("purple dog"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(found.size(), 1);
        const auto found0 = server.FindTopDocuments("purple cat"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL(found0.size(), 1);
        server.AddDocument(doc_id3, content2, DocumentStatus::BANNED, ratings2);
        const auto found1 = server.FindTopDocuments("purple dog"s, DocumentStatus::BANNED);
        ASSERT_EQUAL(found1.size(), 2);
        const auto found2 = server.FindTopDocuments("purple dog"s, DocumentStatus::IRRELEVANT);
        const auto found3 = server.FindTopDocuments("purple dog"s, DocumentStatus::REMOVED);
        ASSERT_EQUAL(found2.size(), 0);
        ASSERT_EQUAL(found3.size(), 0);
    }
}

//Тест проверяет корректное вычисление релевантности найденных документов
void TestComputeDocumentRelevance() {
    const int doc_id = 42;
    const string content = "purple cat purple eyes"s;
    const vector<int> ratings = { 1, 2, 3 };
    const int doc_id2 = 43;
    const string content2 = "small cowardly purple dog"s;
    const vector<int> ratings2 = { 1, -1, 3 };
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto found = server.FindTopDocuments("purple dog");
        ASSERT_EQUAL(found[1].relevance, 0.0);
        ASSERT_EQUAL(found[0].relevance, log(2) * 0.25);
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
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
}