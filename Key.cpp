#include "Key.h"
#include <sstream>

Key::Key(const std::string& key, KeyType type, bool used, const std::string& username)
    : keyValue(key), keyType(type), isUsed(used), discordUsername(username) {
}

std::string Key::getKeyValue() const {
    return keyValue;
}

bool Key::getIsUsed() const {
    return isUsed;
}

std::string Key::getDiscordUsername() const {
    return discordUsername;
}

KeyType Key::getKeyType() const {
    return keyType;
}

std::string Key::getKeyTypeName() const {
    switch (keyType) {
    case KeyType::Day:
        return "Daily";
    case KeyType::Week:
        return "Weekly";
    case KeyType::Month:
        return "Monthly";
    case KeyType::Lifetime:
        return "Lifetime";
    default:
        return "Unknown";
    }
}

void Key::setIsUsed(bool used) {
    isUsed = used;
}

void Key::setDiscordUsername(const std::string& username) {
    discordUsername = username;
}

void Key::setKeyType(KeyType type) {
    keyType = type;
}

std::string Key::serialize() const {
    // Format: keyValue|typeValue|isUsed|discordUsername
    // Using | as separator instead of comma to avoid issues with usernames containing commas
    return keyValue + "|" +
        std::to_string(static_cast<int>(keyType)) + "|" +
        (isUsed ? "1" : "0") + "|" +
        discordUsername;
}

Key Key::deserialize(const std::string& serialized) {
    // Default values
    std::string key = "";
    KeyType type = KeyType::Day;
    bool used = false;
    std::string username = "";

    try {
        // Check for empty input
        if (serialized.empty()) {
            return Key(key, type, used, username);
        }

        // First try to parse with the pipe separator (new format)
        if (deserializeWithSeparator(serialized, '|', key, type, used, username)) {
            return Key(key, type, used, username);
        }

        // If that fails, try with comma separator (old format)
        if (deserializeWithSeparator(serialized, ',', key, type, used, username)) {
            return Key(key, type, used, username);
        }

        // If both formats fail but we have a non-empty string, at least save the key value
        if (!serialized.empty()) {
            key = serialized;
        }
    }
    catch (...) {
        // If any exception occurs, return a key with default values except for the key string
        if (key.empty() && !serialized.empty()) {
            // Try to extract at least the key value
            size_t separatorPos = serialized.find_first_of(",|");
            if (separatorPos != std::string::npos) {
                key = serialized.substr(0, separatorPos);
            }
            else {
                key = serialized;
            }
        }
    }

    return Key(key, type, used, username);
}

// Helper method to try parsing with a specific separator
bool Key::deserializeWithSeparator(const std::string& serialized, char separator,
    std::string& key, KeyType& type,
    bool& used, std::string& username) {
    std::stringstream ss(serialized);
    std::string typeStr, usedStr;

    // Get the key value
    if (!std::getline(ss, key, separator)) {
        return false;
    }

    // Try to get the type (optional)
    if (std::getline(ss, typeStr, separator)) {
        try {
            // Only convert if the string contains digits
            if (std::all_of(typeStr.begin(), typeStr.end(), [](char c) { return std::isdigit(c); })) {
                int typeInt = std::stoi(typeStr);
                // Validate range
                if (typeInt >= 0 && typeInt <= 3) {
                    type = static_cast<KeyType>(typeInt);
                }
            }
        }
        catch (...) {
            // Ignore conversion errors, keep default
        }

        // Try to get the used status (optional)
        if (std::getline(ss, usedStr, separator)) {
            used = (usedStr == "1");

            // Get username (remainder of the string)
            std::getline(ss, username);
        }
    }

    return true;
}