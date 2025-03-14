#ifndef KEYCOLLECTION_H
#define KEYCOLLECTION_H

#include "Key.h"
#include <vector>
#include <string>

// Key collection class to manage multiple keys
class KeyCollection {
private:
    std::vector<Key> keys;

public:
    KeyCollection() = default;

    void addKey(const Key& key);
    bool markKeyAsUsed(size_t index, const std::string& username);
    bool markKeyAsUnused(size_t index);
    std::vector<Key> searchByDiscordUsername(const std::string& username) const;
    std::vector<Key> getAllKeys() const;

    // Serialization for storage
    std::string serialize() const;

    // Deserialization from storage
    static KeyCollection deserialize(const std::string& serialized);

    size_t size() const;
    const Key& at(size_t index) const;
};

#endif // KEYCOLLECTION_H