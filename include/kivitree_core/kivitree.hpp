#pragma once

#include "kivitree_core/lru_store.hpp"
#include <string>
#include <optional>
#include <vector>

class KiviTree{
    private:
        LRUStore engine;
    
    public:
        explicit KiviTree(size_t capacity = 100);

        void put(const std::string& key, const std::string& value);
        std::optional<std::string> get(const std::string& key);
        bool remove(const std::string& key);

        std::unordered_map<std::string, std::string> get_kv_batch() const;
        void set_kv_batch(const std::unordered_map<std::string, std::string>& kvs);

        void print_kivitree();
        void print_kivitree(std::ostream& out) const;
};