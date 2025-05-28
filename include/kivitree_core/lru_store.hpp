#pragma once

#include "kivitree_core/hash_store.hpp"
#include <list>
#include <unordered_map>
#include <optional>
#include <string>
#include <shared_mutex>

class LRUStore {
private:
    size_t capacity;
    HashStore store;
    std::list<std::string> lru_list;
    std::unordered_map<std::string, std::list<std::string>::iterator> index;
    mutable std::shared_mutex mutex;

public:
    LRUStore(size_t capacity):capacity(capacity){};

    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool remove(const std::string& key);

    // batch apis
    std::unordered_map<std::string, std::string> get_kv_batch() const;
    void set_kv_batch(const std::unordered_map<std::string, std::string>& kvs);

    ~LRUStore(){};
};

