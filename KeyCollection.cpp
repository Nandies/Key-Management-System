#include "KeyCollection.h"
#include <algorithm>
#include <sstream>
#include <iostream>

void KeyCollection::addKey(const Key& key) {
    // Skip keys with empty key values
    if (key.getKeyValue().empty()) {
        return;
    }

    // Check if key already exists
    auto it = std::find_if(keys.begin(), keys.end(),
        [&key](const Key& existingKey) {
            return existingKey.getKeyValue() == key.getKeyValue();
        });

    if (it == keys.end()) {
        keys.push_back(key);
    }
}

bool KeyCollection::markKeyAsUsed(size_t index, const std::string& username) {
    if (index >= keys.size()) {
        return false;
    }

    keys[index].setIsUsed(true);
    keys[index].setDiscordUsername(username);
    return true;
}

bool KeyCollection::markKeyAsUnused(size_t index) {
    if (index >= keys.size()) {
        return false;
    }

    keys[index].setIsUsed(false);
    keys[index].setDiscordUsername("");
    return true;
}

std::vector<Key> KeyCollection::searchByDiscordUsername(const std::string& username) const {
    std::vector<Key> results;
    for (const auto& key : keys) {
        if (key.getDiscordUsername().find(username) != std::string::npos) {
            results.push_back(key);
        }
    }
    return results;
}

std::vector<Key> KeyCollection::getAllKeys() const {
    return keys;
}

std::string KeyCollection::serialize() const {
    std::stringstream ss;
    for (const auto& key : keys) {
        ss << key.serialize() << std::endl;
    }
    return ss.str();
}

KeyCollection KeyCollection::deserialize(const std::string& serialized) {
    KeyCollection collection;

    try {
        // Special case for empty input
        if (serialized.empty()) {
            return collection;
        }

        std::stringstream ss(serialized);
        std::string line;
        int lineNumber = 0;
        int validKeys = 0;
        int invalidKeys = 0;

        while (std::getline(ss, line)) {
            lineNumber++;

            // Skip empty lines
            if (line.empty()) {
                continue;
            }

            try {
                Key key = Key::deserialize(line);

                // Only add keys that have a non-empty key value
                if (!key.getKeyValue().empty()) {
                    collection.addKey(key);
                    validKeys++;
                }
                else {
                    invalidKeys++;
                    std::cerr << "Warning: Empty key value at line " << lineNumber << std::endl;
                }
            }
            catch (const std::exception& e) {
                invalidKeys++;
                std::cerr << "Error deserializing key at line " << lineNumber
                    << ": " << e.what() << std::endl;
            }
            catch (...) {
                invalidKeys++;
                std::cerr << "Unknown error deserializing key at line " << lineNumber << std::endl;
            }
        }

        if (invalidKeys > 0) {
            std::cerr << "Loaded " << validKeys << " valid keys. "
                << invalidKeys << " keys were invalid and skipped." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error deserializing key collection: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error deserializing key collection" << std::endl;
    }

    return collection;
}

size_t KeyCollection::size() const {
    return keys.size();
}

const Key& KeyCollection::at(size_t index) const {
    static Key emptyKey(""); // Static fallback for out-of-bounds access

    if (index < keys.size()) {
        return keys.at(index);
    }
    else {
        std::cerr << "Warning: Attempted to access key at invalid index " << index << std::endl;
        return emptyKey;
    }
}