#include "kivitree_core/hash_store.hpp"
#include <unordered_map>

void HashStore::put(const std::string& key, const std::string& value){
    // kv.insert({key, value});
    kv[key] = value;
}

bool HashStore::exists(const std::string& key) const{
    return kv.find(key) != kv.end();
}

bool HashStore::remove(const std::string& key){
    return kv.erase(key) > 0;
}

std::optional<std::string> HashStore::get(const std::string&key) const{
    auto it = kv.find(key);
    if (it != kv.end()){
        return it->second;
    }
    return std::nullopt;
}

std::unordered_map<std::string, std::string> HashStore::get_all() const{
    std::shared_lock lock(mutex);
    return kv;
}