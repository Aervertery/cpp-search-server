#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"

using namespace std; //��������� ������������ ���, ����� ������ ����� ��������. ��������� �������� � ��� �����

int main() {
    SearchServer search_server("and in at"s);
    RequestQueue request_queue(search_server);
    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
    /*r(int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    request_queue.AddFindRequest("curly dog"s);
    request_queue.AddFindRequest("big collar"s);
    request_queue.AddFindRequest("sparrow"s);*/
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    int page_size = 2;
    auto search_results = search_server.FindTopDocuments("big curly cat"s);
    const auto pages = Paginate(search_results, page_size);
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        std::cout << *page << std::endl;
        std::cout << "Page break"s << std::endl;
    }
    return 0;
}

//TODO: ������ GetDocumentId ������� ������������ ��������� ������ begin(), end() ��� ��������� ������� � id ���� ����������
//����� ��������� ������� ���� �� id ��������� const map<string, double>& GetWordFrequencies(int document_id) const �������� id, ���������� ������� ����� -> ���������� ���������� ����� ����� � ��������� / �� ����������
//���� � ���������. ���� ��������� ���, ������� ������ �������
//����� �������� ���������� �� �������  void RemoveDocument(int document_id). ��� �������� �������� documents_freqs_
//� remove_deduplicates.h ������� ����� ��� ������ � �������� ����������  void RemoveDuplicates(SearchServer& search_server). ������ ���������� ������ ����, ���������� ���������� �� ����� ��������, ����-����� ������������. 
//��� �������� ������ ���������� ��������� ���� "Found duplicate document id 7"
//� DocumentData �������� set<string> �� ������� �� ����� ���������
 
