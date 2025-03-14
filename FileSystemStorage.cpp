#include "FileSystemStorage.h"
#include <fstream>
#include <sstream>
#include <iostream>

FileSystemStorage::FileSystemStorage(const std::string& path) : filePath(path) {}

bool FileSystemStorage::saveKeys(const std::string& data) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for writing: " << filePath << std::endl;
            return false;
        }
        file << data;
        file.close();

        // Verify the file was written correctly
        std::ifstream verifyFile(filePath);
        if (!verifyFile.is_open()) {
            std::cerr << "Error: Unable to verify file was written: " << filePath << std::endl;
            return false;
        }
        verifyFile.close();

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving to file: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown error saving to file" << std::endl;
        return false;
    }
}

std::string FileSystemStorage::loadKeys() {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for reading: " << filePath << std::endl;
            return "";
        }

        // Read the file size first
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Check for empty file
        if (size == 0) {
            std::cerr << "Warning: File is empty: " << filePath << std::endl;
            return "";
        }

        // Read file contents with error handling
        std::string contents;
        std::stringstream buffer;

        try {
            buffer << file.rdbuf();
            contents = buffer.str();
        }
        catch (const std::exception& e) {
            std::cerr << "Error reading file contents: " << e.what() << std::endl;
            return "";
        }

        file.close();

        // Check if we actually read anything
        if (contents.empty() && size > 0) {
            std::cerr << "Warning: File size is " << size << " bytes but no content was read" << std::endl;
        }

        return contents;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading from file: " << e.what() << std::endl;
        return "";
    }
    catch (...) {
        std::cerr << "Unknown error loading from file" << std::endl;
        return "";
    }
}

bool FileSystemStorage::exists() {
    try {
        std::ifstream file(filePath);
        bool fileExists = file.good();
        file.close();
        return fileExists;
    }
    catch (const std::exception& e) {
        std::cerr << "Error checking if file exists: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown error checking if file exists" << std::endl;
        return false;
    }
}