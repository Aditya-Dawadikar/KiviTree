#include "kivitree_core/kivitree.hpp"
#include <iostream>
#include <iomanip>
#include <string>

KiviTree::KiviTree(size_t capacity): engine(capacity){}

void KiviTree::put(const std::string& key, const std::string& value) {
    std::cout<<"[INSERT] "<<key<<": "<<value<<"\n";
    engine.put(key, value);
}

std::optional<std::string> KiviTree::get(const std::string& key) {
    return engine.get(key);
}

bool KiviTree::remove(const std::string& key) {
    return engine.remove(key);
}

std::unordered_map<std::string, std::string> KiviTree::get_kv_batch() const{
    return engine.get_kv_batch();
}

void KiviTree::set_kv_batch(const std::unordered_map<std::string, std::string>& kvs){
    engine.set_kv_batch(kvs);
}

void KiviTree::print_kivitree(){
    const int key_width = 20;
    const int val_width = 30;

    auto truncate = [](const std::string& str, int max_len) {
        if (str.length() <= max_len) return str;
        return str.substr(0, max_len - 3) + "...";
    };

    std::cout << "\n| " 
              << std::left << std::setw(key_width) << "Key" 
              << " | " << std::left << std::setw(val_width) << "Value" 
              << " |\n";

    std::cout << "|"
              << std::string(key_width + 2, '-')
              << "|"
              << std::string(val_width + 2, '-')
              << "|\n";

    for (const auto& [key, value] : engine.get_kv_batch()) {
        std::cout << "| " 
                  << std::left << std::setw(key_width) << truncate(key, key_width) 
                  << " | " 
                  << std::left << std::setw(val_width) << truncate(value, val_width) 
                  << " |\n";
    }
    std::cout << std::endl;
}

void KiviTree::print_kivitree(std::ostream& out) const {
    auto kv_pairs = get_kv_batch();

    // Header
    out << "| Key           | Value         |\n";
    out << "|---------------|---------------|\n";

    const int max_len = 15;

    for (const auto& [key, value] : kv_pairs) {
        std::string k = key.substr(0, max_len);
        std::string v = value.substr(0, max_len);

        if (key.length() > max_len) k += "...";
        if (value.length() > max_len) v += "...";

        out << "| " << std::setw(max_len) << std::left << k
            << " | " << std::setw(max_len) << std::left << v << " |\n";
    }

    if (kv_pairs.empty()) {
        out << "| (empty)       |               |\n";
    }
}
