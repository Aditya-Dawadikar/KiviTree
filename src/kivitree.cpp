#include "kivitree.hpp"

KiviTree::KiviTree(size_t capacity): engine(capacity){}

void KiviTree::put(const std::string& key, const std::string& value) {
    engine.put(key, value);
}

std::optional<std::string> KiviTree::get(const std::string& key) {
    return engine.get(key);
}

bool KiviTree::remove(const std::string& key) {
    return engine.remove(key);
}