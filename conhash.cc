#include <map>
#include <vector>
#include <iostream>
#include <set>
#include "pprint.hpp"

template <typename K, typename V>
struct DynamicHash {
    const size_t POINTS_PER_VALUE = 3;

    std::map<V, std::vector<uint64_t>> value_to_points;
    std::map<uint64_t, typename decltype(value_to_points)::iterator> point_to_value;

    DynamicHash(const std::vector<V> & values) {
        for (const auto & v : values) {
            auto r = value_to_points.emplace(v, std::vector<uint64_t>{});
            auto it = r.first;
            for (size_t k = 0; k < POINTS_PER_VALUE; ++k) {
                auto point = redistribute(rand());
                while (point_to_value.find(point) != point_to_value.end()) {
                    point = redistribute(rand());
                }
                point_to_value.emplace(point, it);
                it->second.emplace_back(point);
            }
        }
    }

    void remove(const V & v) {
        auto it = value_to_points.find(v);
        if (it != value_to_points.end()) {
            for (const auto & point : it->second) {
                point_to_value.erase(point);
            }
            value_to_points.erase(it);
        }
    }

    void insert(const V & v) {
        auto it = value_to_points.find(v);
        if (it == value_to_points.end()) {
            auto r = value_to_points.emplace(v, std::vector<uint64_t>{});
            auto it = r.first;
            for (size_t k = 0; k < POINTS_PER_VALUE; ++k) {
                auto point = redistribute(rand());
                while (point_to_value.find(point) != point_to_value.end()) {
                    point = redistribute(rand());
                }
                point_to_value.emplace(point, it);
                it->second.emplace_back(point);
            }
        }
    }


    static uint64_t redistribute(uint64_t x) {
        x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
        x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
        x = x ^ (x >> 31);
        return x;
    }

    V hash(const K & key) {
        if (point_to_value.empty()) {
            throw std::runtime_error("empty cicle");
        }
        auto point = redistribute(std::hash<K>{}(key));
        std::cout << point << ' ';
        auto it = point_to_value.lower_bound(point);
        if (it == point_to_value.end()) {
            it = point_to_value.begin();
        }
        return it->second->first;
    }

    void dump(std::ostream & stream) {
        pprint::PrettyPrinter printer(stream);
        printer.print(point_to_value);
        printer.print(value_to_points);
    }

};

int main() {
    DynamicHash<char, int> dhash(std::vector<int>{1,3,5,7});
    for (int i = 0 ; i < 10; ++i) {
        char k = 'a' + i;
        std::cout << k << ' ' << dhash.hash(k) << std::endl;
    }
    dhash.dump(std::cout);

    dhash.insert(2);
    for (int i = 0 ; i < 10; ++i) {
        char k = 'a' + i;
        std::cout << k << ' ' << dhash.hash(k) << std::endl;
    }
    dhash.dump(std::cout);

    dhash.remove(3);
    for (int i = 0 ; i < 10; ++i) {
        char k = 'a' + i;
        std::cout << k << ' ' << dhash.hash(k) << std::endl;
    }
    dhash.dump(std::cout);
}
