#include "kivitree_core/hash_store.hpp"
#include "kivitree_core/lru_store.hpp"
#include "mutex"

void LRUStore::put(const std::string& key, const std::string& value){
    // if key exists, move it to the front
    if(index.find(key)!=index.end()){
        lru_list.erase(index[key]);
        lru_list.push_front(key);
        index[key] = lru_list.begin();
        
        store.put(key,value);
        return;
    }

    // if capacty is full, evict the key at the end of list
    if(lru_list.size() >= capacity){
        std::string lru_key = lru_list.back();
        lru_list.pop_back();
        index.erase(lru_key);
        store.remove(lru_key);
    }

    // insert new key
    lru_list.push_front(key);
    index[key] = lru_list.begin();
    store.put(key,value);
}

std::optional<std::string> LRUStore::get(const std::string& key){
    if(!store.exists(key)) return std::nullopt;

    // move key to front
    lru_list.erase(index[key]);
    lru_list.push_front(key);
    index[key] = lru_list.begin();

    return store.get(key);

}

bool LRUStore::remove(const std::string& key){
    if(!store.exists(key)) return false;

    lru_list.erase(index[key]);
    index.erase(key);
    return store.remove(key);
}

std::unordered_map<std::string, std::string> LRUStore::get_kv_batch() const{
    std::shared_lock lock(mutex);
    return store.get_all();
}

void LRUStore::set_kv_batch(const std::unordered_map<std::string, std::string>& kvs){
    std::unique_lock lock(mutex);

    for(const auto& [key,value]: kvs){
        put(key,value);
    }
}