#pragma once
#include <vector>
#include <stdexcept>

// Custom HashMap implementation using separate chaining
template <typename K, typename V>
class HashMap {
private:
    struct Entry {
        K key;
        V value;
        Entry* next;
        Entry(const K& k, const V& v) : key(k), value(v), next(nullptr) {}
    };

    std::vector<Entry*> buckets;
    int capacity;
    int size_;

    int hash(int key) const {
        return ((key % capacity) + capacity) % capacity;
    }

public:
    HashMap(int cap = 64) : capacity(cap), size_(0) {
        buckets.resize(capacity, nullptr);
    }

    ~HashMap() {
        for (int i = 0; i < capacity; i++) {
            Entry* cur = buckets[i];
            while (cur) {
                Entry* nxt = cur->next;
                delete cur;
                cur = nxt;
            }
        }
    }

    void insert(const K& key, const V& value) {
        int idx = hash(key);
        Entry* cur = buckets[idx];
        while (cur) {
            if (cur->key == key) { cur->value = value; return; }
            cur = cur->next;
        }
        Entry* e = new Entry(key, value);
        e->next = buckets[idx];
        buckets[idx] = e;
        size_++;
    }

    V* find(const K& key) {
        int idx = hash(key);
        Entry* cur = buckets[idx];
        while (cur) {
            if (cur->key == key) return &cur->value;
            cur = cur->next;
        }
        return nullptr;
    }

    bool contains(const K& key) {
        return find(key) != nullptr;
    }

    int size() const { return size_; }
};
