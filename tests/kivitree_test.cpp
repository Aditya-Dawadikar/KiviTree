#include "kivitree.hpp"
#include <iostream>
#include <cassert>

void test_put_and_get() {
    KiviTree store(3);
    store.put("apple", "red");
    store.put("banana", "yellow");

    assert(store.get("apple").value() == "red");
    assert(store.get("banana").value() == "yellow");

    std::cout << "[PASS] test_put_and_get\n";
}

void test_overwrite_value() {
    KiviTree store(3);
    store.put("key", "old");
    store.put("key", "new");

    assert(store.get("key").value() == "new");

    std::cout << "[PASS] test_overwrite_value\n";
}

void test_eviction_order() {
    KiviTree store(2);
    store.put("A", "1");
    store.put("B", "2");

    // Access A so it becomes MRU
    store.get("A");

    // Add C → should evict B
    store.put("C", "3");

    assert(store.get("A").value() == "1");
    assert(!store.get("B").has_value());
    assert(store.get("C").value() == "3");

    std::cout << "[PASS] test_eviction_order\n";
}

void test_remove_key() {
    KiviTree store(2);
    store.put("temp", "123");
    assert(store.get("temp").value() == "123");

    assert(store.remove("temp"));
    assert(!store.get("temp").has_value());

    std::cout << "[PASS] test_remove_key\n";
}

void test_eviction_on_overflow() {
    KiviTree store(3);
    store.put("1", "one");
    store.put("2", "two");
    store.put("3", "three");

    // All slots are filled, now add one more
    store.put("4", "four");

    // One key should be evicted — LRU is "1"
    assert(!store.get("1").has_value());
    assert(store.get("4").value() == "four");

    std::cout << "[PASS] test_eviction_on_overflow\n";
}

int main() {
    test_put_and_get();
    test_overwrite_value();
    test_eviction_order();
    test_remove_key();
    test_eviction_on_overflow();

    std::cout << "✅ All KiViTree tests passed!\n";
    return 0;
}
