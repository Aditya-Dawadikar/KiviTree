#ifndef TRIE_INDEX_HPP
#define TRIE_INDEX_HPP

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

class TrieIndex{
    private:
        struct Node {
            std::unordered_map<char, Node*> children;
            bool is_terminal = false;
        };

        Node* root = new Node();

    public:
        TrieIndex();

        void insert(const std::string& key);
        void remove(const std::string& key);
        std::optional<std::string> get(const std::string& key) const;
        std::vector<std::string> keys_with_prefix(const std::string& prefix) const;
        bool exists(const std::string&key);
};

#endif