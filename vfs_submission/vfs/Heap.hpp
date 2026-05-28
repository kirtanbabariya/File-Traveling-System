#pragma once
#include <vector>
#include <functional>

// Custom Max-Heap implementation using a comparator
template <typename T>
class Heap {
private:
    std::vector<T> data;
    std::function<bool(const T&, const T&)> cmp; // returns true if a has higher priority than b

    void heapify_up(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (cmp(data[i], data[parent])) {
                std::swap(data[i], data[parent]);
                i = parent;
            } else break;
        }
    }

    void heapify_down(int i) {
        int n = data.size();
        while (true) {
            int best = i;
            int l = 2 * i + 1, r = 2 * i + 2;
            if (l < n && cmp(data[l], data[best])) best = l;
            if (r < n && cmp(data[r], data[best])) best = r;
            if (best == i) break;
            std::swap(data[i], data[best]);
            i = best;
        }
    }

public:
    Heap(std::function<bool(const T&, const T&)> comparator) : cmp(comparator) {}

    void push(const T& val) {
        data.push_back(val);
        heapify_up(data.size() - 1);
    }

    T top() const { return data[0]; }

    void pop() {
        data[0] = data.back();
        data.pop_back();
        if (!data.empty()) heapify_down(0);
    }

    bool empty() const { return data.empty(); }
    int size() const { return data.size(); }

    // Rebuild heap from a new vector (used to refresh after updates)
    void rebuild(const std::vector<T>& items) {
        data = items;
        for (int i = data.size() / 2 - 1; i >= 0; i--)
            heapify_down(i);
    }

    std::vector<T> sorted_copy() const {
        // Returns elements in priority order (destructively on copy)
        Heap<T> tmp(cmp);
        tmp.data = data;
        std::vector<T> result;
        while (!tmp.empty()) {
            result.push_back(tmp.top());
            tmp.pop();
        }
        return result;
    }
};
