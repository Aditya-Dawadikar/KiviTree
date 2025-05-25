#ifndef HASH_STORE_HPP
#define HASH_STORE_HPP

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

class HashStore {
private:
    std::unordered_map<std::string, std::string> kv;
public:
    void put(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);
    bool exists(const std::string& key) const;
};


#endif