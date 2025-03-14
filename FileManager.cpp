#include "FileManager.h"
#include <windows.h>
#include <shlobj.h>

std::string FileManager::getAppDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        std::string appDataPath = std::string(path) + "\\KeyManager\\";
        createDirectoryIfNotExists(appDataPath);
        return appDataPath;
    }
    return ".\\"; // Fallback to current directory
}

void FileManager::createDirectoryIfNotExists(const std::string& path) {
    CreateDirectoryA(path.c_str(), NULL);
}