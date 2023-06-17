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

// ���� ���������, ��� ��������� ������� ��������� ����-����� ��� ���������� ����������
void TestExcludeStopWordsFromAddedDocumentContent();

//���� ��������� ���������� ��������� � ��� ���������� �� �������, ������� ��������
//����� �� ����� ���������
void TestAddingDocument();

//���� ��������� ���������� �� ����������� ������ ����������, ���������� �����-�����
void TestMinusWords();

//���� ��������� ������������ ���������� ���������� �������
void TestDocumentMatching();

//���� ��������� ���������� ��������� ���������� �� �������� �������������
void TestDocumentSorting();

//���� ��������� ���������� �������� ���������
void TestComputeDocumentRating();

//���� ��������� ����� ���������� �� �������-���������, ����������� �������������
void TestDocumentFilterUsingPredicate();

//���� ��������� ����� ���������� �� ��������� �������
void TestDocumentSearchWithCertainStatus();

//���� ��������� ���������� ���������� ������������� ��������� ����������
void TestComputeDocumentRelevance();

//���� ��������� ���������� ���������� ����������� ������ �� ��������
void TestPaginate();

//���� ��������� ���������� ��������� ������� ���� ����������� ���������
void TestGetWordFrequencies();

//���� ��������� ���������� �������� ��������� �� ����
void TestRemoveDocument();

//���� ��������� ���������� �������� ����������-���������� �� ����
void TestRemoveDuplicates();

//���� ��������� ���������� ������ ������� ��������
void TestRequestQueue();

// ������� TestSearchServer �������� ������ ����� ��� ������� ������
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