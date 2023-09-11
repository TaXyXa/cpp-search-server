#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <numeric>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
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
    Document() : id(0), relevance(0), rating(0)
    {}
    Document(int id_, double relevance_, int rating_) {
        id = id_;
        relevance = relevance_;
        rating = rating_;
    }
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:

    inline static constexpr int INVALID_DOCUMENT_ID = -1;
    
    SearchServer() = default; 
    template <typename T>
    SearchServer (const T& conteiner) { 
        for (const auto& word : CheckCont (conteiner)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("некорректные символы");
            }
            stop_words_.insert(word);
        }        
    }  
    
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        if (document_id < 0) {
            throw invalid_argument("id меньше нуля");
        }
        if (documents_.count(document_id)) {
            throw invalid_argument("id уже существует");
        }
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            if (!IsValidWord(word)) {
                throw invalid_argument("некорректные символы");
            }
        }
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        ids_.push_back(document_id);
    }
    //шаблонный
    template <typename Filter>
    vector<Document> FindTopDocuments(const string& raw_query, Filter filter) const {
        const Query query = ParseQuery(raw_query);
        vector<Document> result = FindAllDocuments(query, filter);

        sort(result.begin(), result.end(), [](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance) < numeric_limits<double>::epsilon()) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
        });
        if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
            result.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return result;
    }
    //Для статуса
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status_query) const {
        return FindTopDocuments(raw_query, [&status_query](int document_id, DocumentStatus status, int rating) {return status == status_query; });
    }
    //для запроса без фильтра (только актуальные документы)
    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL;});
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return tuple{matched_words, documents_.at(document_id).status};
    }
    
    int GetDocumentId(int index) const {
        if (index <= 0) {
            throw out_of_range("отрицательный порядковый номер");
        }
        if (index > ids_.size()) {
            throw out_of_range("документов не так много");
        }
        return ids_[index-1];
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector <int> ids_;

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
    
    static bool IsValidWord(const string& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        string word = text;
        if (word[0] == '-') {
            is_minus = true;
            word = text.substr(1);
        }
        if (word.empty() || word[0]=='-' || !IsValidWord(word)) {
            throw invalid_argument("некорректное слово запроса: " + text);
        }
        return {word, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }
    
    template <typename T>
    T CheckCont (const T& conteiner) {
        return conteiner;
    }
    
    vector <string> CheckCont (const string& conteiner) {
        return SplitIntoWords (conteiner);
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename Filter>
    vector<Document> FindAllDocuments(const Query& query, Filter filter) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }

        vector <int> indexs;
        int index = 0;
        for (const auto& document : matched_documents) {
            if (!filter(document.id, documents_.at(document.id).status, document.rating)) {
                indexs.push_back(index);
            }
            ++index;
        }

        for (int i = indexs.size(); i > 0; i--) {
            matched_documents.erase(matched_documents.begin() + indexs[i - 1]);
        }
        return matched_documents;
    }
};

// ==================== для примера =========================
void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}
int main() {
    try {
        SearchServer search_server("и в на"s);
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
        search_server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
        search_server.AddDocument(-1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
        search_server.AddDocument(3, "большой пёс скво\x12рец"s, DocumentStatus::ACTUAL, {1, 3, 2});
        const auto documents = search_server.FindTopDocuments("--пушистый"s);
        for (const Document& document : documents) {
            PrintDocument(document);
        }
    } catch (const invalid_argument& e) {
        cout << "error: "s << e.what() << endl;
    } catch (const out_of_range& e) {
        cout << "error: "s << e.what() << endl;
    }
}

