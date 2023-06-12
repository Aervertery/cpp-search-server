#pragma once
#include "paginator.h"
#include "search_server.h"
#include "remove_duplicates.h"
#include "request_queue.h"
#include "process_queries.h"

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint);

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
    const std::string& hint);

template <typename T>
void RunTestImpl(const T&, const std::string& t_str);

// “ест провер€ет, что поискова€ система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent();

//“ест провер€ет добавление документа и его нахождение по запросу, который содержит
//слова из этого документа
void TestAddingDocument();

//“ест провер€ет исключение из результатов поиска документов, содержащих минус-слова
void TestMinusWords();

//“ест провер€ет соответствие документов поисковому запросу
void TestDocumentMatching();

//“ест провер€ет сортировку найденных документов по убыванию релевантности
void TestDocumentSorting();

//“ест провер€ет вычисление рейтинга документа
void TestComputeDocumentRating();

//“ест провер€ет поиск документов по фильтру-предикату, задаваемому пользователем
void TestDocumentFilterUsingPredicate();

//“ест провер€ет поиск документов по заданному статусу
void TestDocumentSearchWithCertainStatus();

//“ест провер€ет корректное вычисление релевантности найденных документов
void TestComputeDocumentRelevance();

//“ест провер€ет корректное разделение результатов поиска на страницы
void TestPaginate();

//“ест провер€ет корректное получение частоты слов конкретного документа
void TestGetWordFrequencies();

//“ест провер€ет корректное удаление документы из базы
void TestRemoveDocument();

//“ест провер€ет корректное удаление документов-дубликатов из базы
void TestRemoveDuplicates();

//“ест провер€ет корректную работу очереди запросов
void TestRequestQueue();

// ‘ункци€ TestSearchServer €вл€етс€ точкой входа дл€ запуска тестов
void TestSearchServer();

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
    const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cout << std::boolalpha;
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cout << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}



template <typename T>
void RunTestImpl(const T&, const std::string& t_str) {
    std::cerr << t_str << " OK"s << std::endl;
}