//#progma once
#include "document.h"
#include "request_queue.h"

RequestQueue::RequestQueue(const SearchServer& search_server)
    :search_server_(search_server) {
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    std::vector<Document> answer = search_server_.FindTopDocuments(raw_query, status);
    if (answer.empty()) {
        requests_.push_back({ false });
    }
    else {
        requests_.push_back({ true });
    }
    if (requests_.size() > min_in_day_) {
        requests_.pop_front();
    }
    return answer;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    std::vector<Document> answer = search_server_.FindTopDocuments(raw_query);
    if (answer.empty()) {
        requests_.push_back({ false });
    }
    else {
        requests_.push_back({ true });
    }
    if (requests_.size() > min_in_day_) {
        requests_.pop_front();
    }
    return answer;
}

int RequestQueue::GetNoResultRequests() const {
    int count = 0;
    for (auto iter = requests_.begin(); iter != requests_.end(); iter++) {
        if (!iter->is_result) {
            ++count;
        }
    }
    return count;
}
