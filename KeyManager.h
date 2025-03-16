#ifndef KEYMANAGER_H
#define KEYMANAGER_H

#include "KeyCollection.h"
#include "IKeyStorage.h"
#include <memory>
#include <string>

// KeyManager class to orchestrate the key management system
class KeyManager {
private:
    KeyCollection m_keyCollection;
    std::unique_ptr<IKeyStorage> storage;

    void saveKeys();

public:
    KeyManager();

    // Added method to get all keys from the collection
    std::vector<Key> getAllKeys() const {
        return m_keyCollection.getAllKeys();
    }

    // Added method to add a key to the collection
    void addKey(const Key& key) {
        m_keyCollection.addKey(key);
        saveKeys();
    }

    // Added method to mark a key as used by its value
    bool markKeyByValue(const std::string& keyValue, const std::string& discordUsername) {
        auto keys = m_keyCollection.getAllKeys();
        for (size_t i = 0; i < keys.size(); i++) {
            if (keys[i].getKeyValue() == keyValue) {
                bool result = m_keyCollection.markKeyAsUsed(i, discordUsername);
                if (result) saveKeys();
                return result;
            }
        }
        return false;
    }

    bool markKeyAsUnusedByValue(const std::string& keyValue) {
        auto keys = m_keyCollection.getAllKeys();
        for (size_t i = 0; i < keys.size(); i++) {
            if (keys[i].getKeyValue() == keyValue && keys[i].getIsUsed()) {
                bool result = m_keyCollection.markKeyAsUnused(i);
                if (result) saveKeys();
                return result;
            }
        }
        return false;
    }

    void importKeysFromFile(const std::string& filename, KeyType keyType);
    void displayKeys() const;
    void displayKeysByType(KeyType keyType) const;
    void markKeyAsUsed();
    void markKeyAsUnused();
    void searchByDiscordUsername() const;
    void displayKeyStatistics() const;
};

#endif // KEYMANAGER_H