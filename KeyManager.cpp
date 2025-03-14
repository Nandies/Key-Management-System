#include "KeyManager.h"
#include "FileSystemStorage.h"
#include "FileManager.h"
#include "KeyImporter.h"
#include <iostream>
#include <map>
#include <algorithm>

KeyManager::KeyManager() {
    try {
        std::string storagePath = FileManager::getAppDataPath() + "keys.csv";
        storage = std::make_unique<FileSystemStorage>(storagePath);

        if (storage->exists()) {
            std::string serialized = storage->loadKeys();
            m_keyCollection = KeyCollection::deserialize(serialized);
            std::cout << "Loaded existing key storage with " << m_keyCollection.size() << " keys." << std::endl;
        }
        else {
            std::cout << "No existing key storage found. A new one will be created." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error initializing storage: " << e.what() << std::endl;
        std::cout << "Starting with empty key collection." << std::endl;
    }
}

void KeyManager::importKeysFromFile(const std::string& filename, KeyType keyType) {
    try {
        auto importedKeysValues = KeyImporter::importFromFile(filename);
        int newKeysCount = 0;

        for (const auto& keyValue : importedKeysValues) {
            if (!keyValue.empty()) {
                // Create a Key object with the specified type
                Key key(keyValue, keyType);

                size_t previousSize = m_keyCollection.size();
                m_keyCollection.addKey(key);
                if (m_keyCollection.size() > previousSize) {
                    newKeysCount++;
                }
            }
        }

        if (newKeysCount > 0) {
            std::cout << "Imported " << newKeysCount << " new keys of type " <<
                Key(std::string(), keyType).getKeyTypeName() << "." << std::endl;
            saveKeys();
        }
        else {
            std::cout << "No new keys imported." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void KeyManager::displayKeys() const {
    if (m_keyCollection.size() == 0) {
        std::cout << "No keys available." << std::endl;
        return;
    }

    std::cout << "\n--- KEY LIST ---" << std::endl;
    std::cout << "Index | Key | Type | Status | Discord Username" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;

    auto keys = m_keyCollection.getAllKeys();
    for (size_t i = 0; i < keys.size(); i++) {
        std::cout << i + 1 << " | "
            << keys[i].getKeyValue() << " | "
            << keys[i].getKeyTypeName() << " | "
            << (keys[i].getIsUsed() ? "Used" : "Available") << " | "
            << keys[i].getDiscordUsername() << std::endl;
    }
    std::cout << "-----------------------------------------------------" << std::endl;
}

void KeyManager::displayKeysByType(KeyType keyType) const {
    if (m_keyCollection.size() == 0) {
        std::cout << "No keys available." << std::endl;
        return;
    }

    // Get all keys of the specified type
    auto allKeys = m_keyCollection.getAllKeys();
    std::vector<Key> filteredKeys;

    for (const auto& key : allKeys) {
        if (key.getKeyType() == keyType) {
            filteredKeys.push_back(key);
        }
    }

    if (filteredKeys.empty()) {
        std::cout << "No keys found with type " << Key(std::string(), keyType).getKeyTypeName() << std::endl;
        return;
    }

    std::cout << "\n--- " << Key(std::string(), keyType).getKeyTypeName() << " KEYS ---" << std::endl;
    std::cout << "Index | Key | Status | Discord Username" << std::endl;
    std::cout << "-----------------------------------------------------" << std::endl;

    for (size_t i = 0; i < filteredKeys.size(); i++) {
        std::cout << i + 1 << " | "
            << filteredKeys[i].getKeyValue() << " | "
            << (filteredKeys[i].getIsUsed() ? "Used" : "Available") << " | "
            << filteredKeys[i].getDiscordUsername() << std::endl;
    }
    std::cout << "-----------------------------------------------------" << std::endl;
}

void KeyManager::markKeyAsUsed() {
    if (m_keyCollection.size() == 0) {
        std::cout << "No keys available." << std::endl;
        return;
    }

    displayKeys();

    int index;
    std::cout << "Enter the index of the key to mark as used: ";

    // Safely get user input
    try {
        std::cin >> index;
        std::cin.ignore(); // Clear the newline character

        if (std::cin.fail()) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(10000, '\n'); // Discard input
            throw std::runtime_error("Invalid input");
        }
    }
    catch (std::exception&) {
        std::cout << "Invalid input. Please enter a number." << std::endl;
        return;
    }

    if (index < 1 || index > static_cast<int>(m_keyCollection.size())) {
        std::cout << "Invalid index." << std::endl;
        return;
    }

    // Adjust index to 0-based
    index--;

    try {
        const Key& key = m_keyCollection.at(index);
        if (key.getIsUsed()) {
            std::cout << "This key is already marked as used by: " << key.getDiscordUsername() << std::endl;
            std::cout << "Do you want to update the Discord username? (y/n): ";
            char choice;
            std::cin >> choice;
            std::cin.ignore(); // Clear the newline character

            if (choice != 'y' && choice != 'Y') {
                return;
            }
        }

        std::string username;
        std::cout << "Enter Discord username: ";
        std::getline(std::cin, username);

        m_keyCollection.markKeyAsUsed(index, username);

        std::cout << "Key marked as used by " << username << std::endl;
        saveKeys();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void KeyManager::markKeyAsUnused() {
    if (m_keyCollection.size() == 0) {
        std::cout << "No keys available." << std::endl;
        return;
    }

    displayKeys();

    int index;
    std::cout << "Enter the index of the key to mark as unused: ";

    // Safely get user input
    try {
        std::cin >> index;

        if (std::cin.fail()) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(10000, '\n'); // Discard input
            throw std::runtime_error("Invalid input");
        }
    }
    catch (std::exception&) {
        std::cout << "Invalid input. Please enter a number." << std::endl;
        return;
    }

    if (index < 1 || index > static_cast<int>(m_keyCollection.size())) {
        std::cout << "Invalid index." << std::endl;
        return;
    }

    // Adjust index to 0-based
    index--;

    try {
        m_keyCollection.markKeyAsUnused(index);
        std::cout << "Key marked as unused." << std::endl;
        saveKeys();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void KeyManager::searchByDiscordUsername() const {
    if (m_keyCollection.size() == 0) {
        std::cout << "No keys available." << std::endl;
        return;
    }

    std::cin.ignore(); // Clear any previous input
    std::string username;
    std::cout << "Enter Discord username to search for: ";
    std::getline(std::cin, username);

    auto results = m_keyCollection.searchByDiscordUsername(username);

    std::cout << "\n--- SEARCH RESULTS ---" << std::endl;

    if (results.empty()) {
        std::cout << "No keys found for the specified Discord username." << std::endl;
    }
    else {
        std::cout << "Key | Type | Status | Discord Username" << std::endl;
        std::cout << "-----------------------------------------------------" << std::endl;

        for (const auto& key : results) {
            std::cout << key.getKeyValue() << " | "
                << key.getKeyTypeName() << " | "
                << (key.getIsUsed() ? "Used" : "Available") << " | "
                << key.getDiscordUsername() << std::endl;
        }
    }

    std::cout << "-----------------------------------------------------" << std::endl;
}

void KeyManager::displayKeyStatistics() const {
    if (m_keyCollection.size() == 0) {
        std::cout << "No keys available." << std::endl;
        return;
    }

    auto allKeys = m_keyCollection.getAllKeys();

    // Count keys by type
    std::map<KeyType, int> typeCounts;
    std::map<KeyType, int> usedTypeCounts;

    for (const auto& key : allKeys) {
        typeCounts[key.getKeyType()]++;
        if (key.getIsUsed()) {
            usedTypeCounts[key.getKeyType()]++;
        }
    }

    std::cout << "\n--- KEY STATISTICS ---" << std::endl;
    std::cout << "Total keys: " << allKeys.size() << std::endl;

    // Display stats for each key type
    const KeyType types[] = { KeyType::Day, KeyType::Week, KeyType::Month, KeyType::Lifetime };

    for (const auto& type : types) {
        std::string typeName = Key(std::string(), type).getKeyTypeName();
        int total = typeCounts[type];
        int used = usedTypeCounts[type];
        int available = total - used;

        if (total > 0) {
            std::cout << "\n" << typeName << " keys:" << std::endl;
            std::cout << "  Total: " << total << std::endl;
            std::cout << "  Used: " << used << " (" << (total > 0 ? (used * 100 / total) : 0) << "%)" << std::endl;
            std::cout << "  Available: " << available << " (" << (total > 0 ? (available * 100 / total) : 0) << "%)" << std::endl;
        }
    }

    int totalUsed = usedTypeCounts[KeyType::Day] +
        usedTypeCounts[KeyType::Week] +
        usedTypeCounts[KeyType::Month] +
        usedTypeCounts[KeyType::Lifetime];

    int totalAvailable = allKeys.size() - totalUsed;

    std::cout << "\nTotal used keys: " << totalUsed;
    if (allKeys.size() > 0) {
        std::cout << " (" << (totalUsed * 100 / allKeys.size()) << "%)";
    }
    std::cout << std::endl;

    std::cout << "Total available keys: " << totalAvailable;
    if (allKeys.size() > 0) {
        std::cout << " (" << (totalAvailable * 100 / allKeys.size()) << "%)";
    }
    std::cout << std::endl;
}

void KeyManager::saveKeys() {
    try {
        std::string serialized = m_keyCollection.serialize();
        if (storage->saveKeys(serialized)) {
            std::cout << "Keys saved successfully!" << std::endl;
        }
        else {
            std::cerr << "Error: Failed to save keys." << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving keys: " << e.what() << std::endl;
    }
}