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
        const vector<string> words = SplitIntoWordsNoStop(document);
        int w_size = words.size();
        for (auto word : words) {
            double tf_value = 1. / w_size;
            documents_[word].insert({ document_id, tf_value });
        }
    }

    void SetDocumentCount(int document_count) {
        document_count_ = document_count;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const set<string> query_words = ParseQuery(raw_query);
        const set<string> minus_words = ParseQueryMinus(raw_query);
        auto matched_documents = FindAllDocuments(query_words, minus_words);

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

    int document_count_ = 0;
    //использую контейнер вида слово - {id, TF}
    map <string, map<int, double>> documents_;

    set<string> stop_words_;

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

    set<string> ParseQuery(const string& text) const {
        set<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] != '-') {
                query_words.insert(word);
            }
        }
        return query_words;
    }

    set<string> ParseQueryMinus(const string& text) const {
        set<string> minus_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-') {
                string ord;
                ord.assign(word.begin() + 1, word.end());
                minus_words.insert(ord);
            }
        }
        return minus_words;
    }

    vector<Document> FindAllDocuments(const set<string>& query_words, const set<string>& minus_words) const {
        vector<Document> matched_documents;
        map <int, double> id_relevant;
        if (!query_words.empty()) {
            for (const auto& word : query_words) {
                if (documents_.count(word) != 0) {
                    for (const auto& [ids, tf_value] : documents_.at(word)) {
                        double doc_size = documents_.at(word).size();
                        double division = document_count_ / doc_size;
                        id_relevant[ids] += tf_value * log(division);
                    }
                }
            }
        }
        if (!minus_words.empty()) {
            for (const auto& minus : minus_words) {
                if (documents_.count(minus) != 0) {
                    for (const auto& [ids, tf_value] : documents_.at(minus)) {
                        id_relevant.erase(ids);
                    }
                }
            }
        }
        for (const auto& id_rel : id_relevant) {
            matched_documents.push_back({ id_rel.first, id_rel.second });
        }
        return matched_documents;
    }
    //end class
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    int document_count = ReadLineWithNumber();
    search_server.SetDocumentCount(document_count);
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
