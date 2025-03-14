#ifndef KEY_H
#define KEY_H

#include <string>
#include <algorithm>

// Enum for key subscription types
enum class KeyType {
    Day,
    Week,
    Month,
    Lifetime
};

// Key class to represent individual license keys
class Key {
private:
    std::string keyValue;
    bool isUsed;
    std::string discordUsername;
    KeyType keyType;

    // Helper for deserialization
    static bool deserializeWithSeparator(const std::string& serialized, char separator,
        std::string& key, KeyType& type,
        bool& used, std::string& username);

public:
    Key(const std::string& key, KeyType type = KeyType::Day, bool used = false, const std::string& username = "");

    // Getters
    std::string getKeyValue() const;
    bool getIsUsed() const;
    std::string getDiscordUsername() const;
    KeyType getKeyType() const;
    std::string getKeyTypeName() const;

    // Setters
    void setIsUsed(bool used);
    void setDiscordUsername(const std::string& username);
    void setKeyType(KeyType type);

    // Serialization for storage
    std::string serialize() const;

    // Deserialization from storage
    static Key deserialize(const std::string& serialized);
};

#endif // KEY_H