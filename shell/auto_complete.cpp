#include "auto_complete.h"
#include <dirent.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

class AutoComplete {
private:
    Trie trie;

    void updateFilesInTrie() {
        trie = Trie();

        DIR* dir = opendir(".");
        if (!dir) {
            perror("opendir");
            return;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            string name = entry->d_name;
            if (name != "." && name != "..") {
                trie.insert(name);
            }
        }
        closedir(dir);
    }

public:
    AutoComplete() {
        updateFilesInTrie();
    }

    void refreshTrie() {
        updateFilesInTrie();
    }

    vector<string> getSuggestions(const string& prefix) {
        return trie.findSuggestions(prefix);
    }

    void displaySuggestions(string& input, string &shell) {
        unsigned long long pos = input.find_last_of(' ');
        string prefix = (pos != string::npos) ? input.substr(pos + 1) : input;

        vector<string> suggestions = getSuggestions(prefix);

        if (!suggestions.empty()) {
            printf("\n");
            for (const string& match : suggestions) {
                printf("%s  ", match.c_str());
            }
            printf("\n%s%s", shell.c_str(), input.c_str());
        }
    }

};
