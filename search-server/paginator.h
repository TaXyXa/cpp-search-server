#pragma once

#include <iostream>

#include "document.h"

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator range_begin, Iterator range_end) {
        int number_doc = distance(range_begin, range_end);
        for (int i = 0; i < number_doc; i++) {
            page_.push_back(*(range_begin + i));
        }
    }
    Iterator begin() {
        return page_.begin();
    }

    Iterator end() {
        return page_.end();
    }

    int size() {
        return distance(page_.begin(), page_.end());
    }

private:
    std::vector <Document> page_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, IteratorRange<Iterator> page) {
    for (auto doc = page.begin(); doc != page.end(); ++doc) {
        out << *doc;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, Document doc) {
    out << "{ document_id = " << doc.id << ", relevance = "
        << doc.relevance << ", rating = " << doc.rating << " }";
    return out;
}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator doc_begin, Iterator doc_end, int page_size) {
        auto iter = doc_begin;
        while (iter < (doc_end - page_size)) {
            pages_.push_back(IteratorRange(iter, iter + page_size));
            iter += page_size;
        }
        pages_.push_back(IteratorRange(iter, doc_end));
    }

    auto begin() const {
        return pages_.begin();
    }

    auto end() const {
        return pages_.end();
    }

private:
    std::vector <IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
