#include "KeyImporter.h"
#include <fstream>
#include <stdexcept>

std::vector<std::string> KeyImporter::importFromFile(const std::string& filename) {
    std::vector<std::string> importedKeys;
    std::ifstream file(filename);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (!line.empty()) {
            importedKeys.push_back(line);
        }
    }

    file.close();
    return importedKeys;
}