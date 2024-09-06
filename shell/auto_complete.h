#ifndef TRIE_H
#define TRIE_H

#include <unordered_map>
#include <vector>
#include <string>

class TrieNode {
public:
    bool is_end_of_word;
    std::unordered_map<char, TrieNode*> child;

    TrieNode() : is_end_of_word(false) {}
};

class Trie {
private:
    TrieNode* root;

    void collectSuggestions(TrieNode* node, std::string word, std::vector<std::string>& suggestions) {
        if (node->is_end_of_word) {
            suggestions.push_back(word);
        }
        for (auto& child : node->child) {
            collectSuggestions(child.second, word + child.first, suggestions);
        }
    }

public:
    Trie() : root(new TrieNode()) {}

    void insert(const std::string& word) {
        TrieNode* curr = root;
        for (char c : word) {
            if (!curr->child.count(c)) {
                curr->child[c] = new TrieNode();
            }
            curr = curr->child[c];
        }
        curr->is_end_of_word = true;
    }
    std::vector<std::string> findSuggestions(const std::string& word) {
        TrieNode* current = root;
        std::vector<std::string> suggestions;

        for (char c : word) {
            if (current->child.find(c) == current->child.end()) {
                return suggestions;
            }
        current = current->child[c];
    }

        collectSuggestions(current, word, suggestions);
        return suggestions;
}

};

#endif
