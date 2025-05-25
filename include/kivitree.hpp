#pragma once

#include "lru_store.hpp"
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
};