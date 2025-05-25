#ifndef HASH_STORE_HPP
#define HASH_STORE_HPP

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <shared_mutex>

class HashStore {
private:
    std::unordered_map<std::string, std::string> kv;
    mutable std::shared_mutex mutex;
public:
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);
    bool exists(const std::string& key) const;

    std::unordered_map<std::string, std::string> get_all() const;
    // void set_all(std::unordered_map<std::string, std::string> data) const;
};


#endif