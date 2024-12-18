#include <iostream>
#include <string>

#include "document.h"
#include "paginator.h"
#include "read_input_functions.h"
#include "request_queue.h"
#include "search_server.h"
#include "string_processing.h"

using namespace std;

int main() {
    SearchServer search_server("with and in at");
    RequestQueue request_queue(search_server);
    search_server.AddDocument(6, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(7, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(8, "big cat nasty hair"s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(9, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(10, "big dog hamster Borya"s, DocumentStatus::ACTUAL, {1, 1, 1});

    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request");
    }
    request_queue.AddFindRequest("curly dog");
    request_queue.AddFindRequest("big collar");

    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request");
    }
    request_queue.AddFindRequest("curly dog");
    request_queue.AddFindRequest("big collar");
    request_queue.AddFindRequest("sparrow");
    std::cout << "Total empty requests: " << request_queue.GetNoResultRequests() << std::endl;
    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    int page_size = 2;
    const auto pages = Paginate(search_results, page_size);
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        cout << *page << endl;
        cout << "Page break"s << endl;
    }
    return 0;
}