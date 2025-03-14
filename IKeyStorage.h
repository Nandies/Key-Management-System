#ifndef IKEYSTORAGE_H
#define IKEYSTORAGE_H

#include <string>

// Interface for key storage
class IKeyStorage {
public:
	virtual ~IKeyStorage() = default;
	virtual bool saveKeys(const std::string& data) = 0;
	virtual std::string loadKeys() = 0;
	virtual bool exists() = 0;
};

#endif // IKEYSTORAGE_H