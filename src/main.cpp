#include "kivitree.hpp"
#include <iostream>
#include <cassert>

int main() {
    KiviTree store(3); // Small capacity to test eviction logic

    // Test basic put/get
    store.put("A", "Alpha");
    store.put("B", "Bravo");
    store.put("C", "Charlie");

    assert(store.get("A").value() == "Alpha");
    assert(store.get("B").value() == "Bravo");
    assert(store.get("C").value() == "Charlie");

    std::cout << "Initial inserts passed ✅\n";

    // Access B to make it MRU
    store.get("B");

    // Add D → should evict A (least recently used)
    store.put("D", "Delta");

    assert(!store.get("A").has_value());  // A should be evicted
    assert(store.get("B").value() == "Bravo");
    assert(store.get("C").value() == "Charlie");
    assert(store.get("D").value() == "Delta");

    std::cout << "LRU eviction logic passed ✅\n";

    // Remove C
    assert(store.remove("C"));
    assert(!store.get("C").has_value());

    std::cout << "Remove logic passed ✅\n";

    // Overwrite B
    store.put("B", "Beta");
    assert(store.get("B").value() == "Beta");

    std::cout << "Overwrite logic passed ✅\n";

    std::cout << "All KiViTree tests passed 🎉\n";
    return 0;
}
