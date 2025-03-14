#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <string>

// FileManager Class to handle system paths and dirs
class FileManager {
public:
	static std::string getAppDataPath();
	static void createDirectoryIfNotExists(const std::string& path);
};

#endif // FILEMANAGER_H