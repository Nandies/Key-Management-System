#ifndef FILESYSTEMSTORAGE_H
#define FILESYSTEMSTORAGE_H

#include "IKeyStorage.h"
#include <string>

// Class for local file system storage
class FileSystemStorage : public IKeyStorage {
private:
    std::string filePath;

public:
    FileSystemStorage(const std::string& path);

    bool saveKeys(const std::string& data) override;
    std::string loadKeys() override;
    bool exists() override;
};

#endif // FILESYSTEMSTORAGE_H