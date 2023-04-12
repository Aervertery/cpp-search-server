#include "test_example_functions.h"

using namespace std; //��������� ������������ ���, ����� ������ ����� ��������. ��������� �������� � ��� �����

int main() {
    SearchServer search_server("and with"s);

    AddDocument(search_server, 10, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // �������� ��������� 2, ����� �����
    AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ������� ������ � ����-������, ������� ����������
    AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ��������� ���� ����� ��, ������� ���������� ��������� 1
    AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ���������� ����� �����, ���������� �� ��������
    AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ��������� ���� ����� ��, ��� � id 6, �������� �� ������ �������, ������� ����������
    AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ���� �� ��� �����, �� �������� ����������
    AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });

    // ����� �� ������ ����������, �� �������� ����������
    AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });

    AddDocument(search_server, 11, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });


    int page_size = 2;
    auto search_results = search_server.FindTopDocuments("funny curly rat"s);
    const auto pages = Paginate(search_results, page_size);
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        std::cout << *page << std::endl;
        std::cout << "Page break"s << std::endl;
    }

    std::cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
    RemoveDuplicates(search_server);
    std::cout << "After duplicates removed: "s << search_server.GetDocumentCount() << std::endl;


    auto search_results2 = search_server.FindTopDocuments("funny curly rat"s);
    const auto pages2 = Paginate(search_results2, page_size);
    for (auto page = pages2.begin(); page != pages2.end(); ++page) {
        std::cout << *page << std::endl;
        std::cout << "Page break"s << std::endl;
    }
}