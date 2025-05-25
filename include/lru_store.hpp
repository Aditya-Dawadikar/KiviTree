#pragma once

#include "hash_store.hpp"
#include <list>
#include <unordered_map>
#include <optional>
#include <string>

class LRUStore {
private:
    size_t capacity;
    HashStore store;
    std::list<std::string> lru_list;
    std::unordered_map<std::string, std::list<std::string>::iterator> index;

public:
    LRUStore(size_t capacity):capacity(capacity){};

    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool remove(const std::string& key);

    ~LRUStore(){};
};

