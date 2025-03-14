#ifndef KEYIMPORTER_H
#define KEYIMPORTER_H

#include <vector>
#include <string>

// KeyImporter class to handle importing keys from external files
class KeyImporter {
public:
    static std::vector<std::string> importFromFile(const std::string& filename);
};

#endif // KEYIMPORTER_H