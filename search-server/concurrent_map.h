#pragma once
#include <mutex>

#include "log_duration.h"
using std::literals::string_literals::operator""s;

template <typename Key, typename Value>
class ConcurrentMap {
private:
    struct Bucket {
        std::mutex mutex;
        std::map<Key, Value> map;
    };

    std::vector<Bucket> buckets_;

    Bucket& GetBucket(const Key& key) {
        return buckets_[static_cast<uint64_t>(key) % buckets_.size()];
    }

public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    using Iterator = Value*;
    using ConstIterator = const Value*;

    struct Access {
        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;

        Access(const Key& key, Bucket& bucket) :
            guard(bucket.mutex),
            ref_to_value(bucket.map[key]) {
        }
    };

    explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count) {
    }

    Access operator[](const Key& key) {
        auto& bucket = GetBucket(key);
        return { key, bucket };
    }

    size_t erase(const Key& key) {
        Bucket& bucket = GetBucket(key);
        bucket.map.erase(key);
        return 0;
    }

    std::map<Key, Value> BuildOrdinaryMap() {
        LOG_DURATION("BuildMap"s);
        std::map<Key, Value> result;
        for (auto& [mutex, map] : buckets_) {
            std::lock_guard guard(mutex);
            result.insert(map.begin(), map.end());
        }
        return result;
    }
};