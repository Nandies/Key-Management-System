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

    void importKeysFromFile(const std::string& filename, KeyType keyType);
    void displayKeys() const;
    void displayKeysByType(KeyType keyType) const;
    void markKeyAsUsed();
    void markKeyAsUnused();
    void searchByDiscordUsername() const;
    void displayKeyStatistics() const;
};

#endif // KEYMANAGER_H