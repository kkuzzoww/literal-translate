#include <__config>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <sqlite3.h>

bool upload_dictionary(sqlite3* db, std::unordered_map<std::string, std::string> &dictionary) {
    const char* sql = "SELECT english, russian FROM translations";
    sqlite3_stmt* stmt;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "failed to prepare statement: " <<sqlite3_errmsg(db) << std::endl;
        return false;
    }

    while(sqlite3_step(stmt) == SQLITE_ROW) {
        std::string english = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string russian = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        dictionary[english] = russian;
    }

    sqlite3_finalize(stmt);

    return true;
}

bool add_to_database(sqlite3* db, const std::string& english, const std::string& russian) {

    const char* sql = "INSERT INTO translations (english, russian) VALUES (?, ?)";
    sqlite3_stmt* stmt;

    if(sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "statement preparation failed: " << sqlite3_errmsg(db) << '\n';
        return false;
    }

    sqlite3_bind_text(stmt, 1, english.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, russian.c_str(), -1, SQLITE_STATIC);

    if(sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "statement execution failed: " << sqlite3_errmsg(db) << '\n';
        sqlite3_finalize(stmt);
    }


    sqlite3_finalize(stmt);
    return true;
}

std::string translate_or_add(const std::string &word,
                             std::unordered_map<std::string, std::string>& dictionary,
                             sqlite3* db) {

    auto it = dictionary.find(word);
    if(it != dictionary.end()) {
        return it->second;
    }

    std::string russian;
    std::cout << "the word ->" << word << "<- is not in the dictionary. please enter the translation" << '\n';
    std::getline(std::cin, russian);

    dictionary[word] = russian;

    if(!add_to_database(db, word, russian)) {
        std::cerr << "failed to add the word into database" << '\n';
    }

    return "";
}

std::string add_phrase(const std::string &word,
                       std::unordered_map<std::string, std::string>& dicitonary,
                       sqlite3 *db) {



}

int main() {

    sqlite3* db;
    std::unordered_map<std::string, std::string> dictionary;

    if(sqlite3_open("dictionary.db", &db) != SQLITE_OK) {
        std::cerr << "failed to open your database: " <<sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    if(!upload_dictionary(db, dictionary)) {
        std::cerr << "failed to load dicitonary" << std::endl;
        sqlite3_close(db);
        return -1;
    }

    while (true) {
        std::string sentence;
        std::cout << "enter your phrase in english to translate it (q to end the programm)\n";
        std::getline(std::cin, sentence);

        if(sentence == "q") break;

        std::stringstream ss(sentence);
        std::string word;
        std::string translated_result;

        while(ss >> word) {
            translated_result += translate_or_add(word, dictionary, db) + " ";
        }

        std::cout << "translated result: " << translated_result << '\n';

    }

    return 0;

}
