#include "BackupRestoreUtil.h"
#include "FileManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <ctime>

bool BackupRestoreUtil::backupDatabase(const std::string& filename) {
    try {
        // Get paths
        std::string appDataPath = FileManager::getAppDataPath();
        std::string databasePath = appDataPath + "keys.csv";
        std::string backupPath = filename;

        // Check if source file exists
        std::ifstream sourceFile(databasePath);
        if (!sourceFile.is_open()) {
            std::cerr << "Error: Cannot open database file for backup: " << databasePath << std::endl;
            return false;
        }

        // Read source file
        std::stringstream buffer;
        buffer << sourceFile.rdbuf();
        sourceFile.close();

        // Write to backup file
        std::ofstream backupFile(backupPath);
        if (!backupFile.is_open()) {
            std::cerr << "Error: Cannot open backup file for writing: " << backupPath << std::endl;
            return false;
        }

        backupFile << buffer.str();
        backupFile.close();

        std::cout << "Database successfully backed up to: " << backupPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during backup: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown error during backup" << std::endl;
        return false;
    }
}

bool BackupRestoreUtil::restoreDatabase(const std::string& filename) {
    try {
        // Get paths
        std::string appDataPath = FileManager::getAppDataPath();
        std::string databasePath = appDataPath + "keys.csv";
        std::string backupPath = filename;

        // Check if backup file exists
        std::ifstream backupFile(backupPath);
        if (!backupFile.is_open()) {
            std::cerr << "Error: Cannot open backup file: " << backupPath << std::endl;
            return false;
        }

        // Read backup file
        std::stringstream buffer;
        buffer << backupFile.rdbuf();
        backupFile.close();

        // Create backup of current database before overwriting
        std::time_t now = std::time(nullptr);
        std::string timestamp = std::to_string(now);
        std::string autoBackupPath = appDataPath + "keys_auto_backup_" + timestamp + ".csv";

        std::ifstream currentDb(databasePath);
        if (currentDb.is_open()) {
            std::ofstream autoBackup(autoBackupPath);
            if (autoBackup.is_open()) {
                autoBackup << currentDb.rdbuf();
                autoBackup.close();
                std::cout << "Created automatic backup of current database: " << autoBackupPath << std::endl;
            }
            currentDb.close();
        }

        // Write to database file
        std::ofstream dbFile(databasePath);
        if (!dbFile.is_open()) {
            std::cerr << "Error: Cannot open database file for writing: " << databasePath << std::endl;
            return false;
        }

        dbFile << buffer.str();
        dbFile.close();

        std::cout << "Database successfully restored from: " << backupPath << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during restore: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown error during restore" << std::endl;
        return false;
    }
}

void BackupRestoreUtil::repairDatabase() {
    try {
        // Get paths
        std::string appDataPath = FileManager::getAppDataPath();
        std::string databasePath = appDataPath + "keys.csv";

        // Check if database file exists
        std::ifstream dbFile(databasePath);
        if (!dbFile.is_open()) {
            std::cout << "No database file found to repair." << std::endl;
            return;
        }

        // Create backup before repair
        std::time_t now = std::time(nullptr);
        std::string timestamp = std::to_string(now);
        std::string backupPath = appDataPath + "keys_before_repair_" + timestamp + ".csv";

        std::ofstream backupFile(backupPath);
        if (backupFile.is_open()) {
            backupFile << dbFile.rdbuf();
            backupFile.close();
            std::cout << "Created backup before repair: " << backupPath << std::endl;
        }

        // Reset file position
        dbFile.clear();
        dbFile.seekg(0);

        // Read and validate each line
        std::vector<std::string> validLines;
        std::string line;
        int lineNumber = 0;
        int validCount = 0;
        int invalidCount = 0;

        while (std::getline(dbFile, line)) {
            lineNumber++;

            // Skip empty lines
            if (line.empty()) {
                continue;
            }

            // Basic validation - check if the line has at least one separator
            if (line.find(',') != std::string::npos || line.find('|') != std::string::npos) {
                validLines.push_back(line);
                validCount++;
            }
            else {
                std::cout << "Line " << lineNumber << " appears invalid and will be skipped." << std::endl;
                invalidCount++;
            }
        }

        dbFile.close();

        // Write back only valid lines
        std::ofstream repairedFile(databasePath);
        if (!repairedFile.is_open()) {
            std::cerr << "Error: Cannot open database file for writing after repair." << std::endl;
            return;
        }

        for (const auto& validLine : validLines) {
            repairedFile << validLine << std::endl;
        }

        repairedFile.close();

        std::cout << "Database repair complete." << std::endl;
        std::cout << "Valid entries: " << validCount << std::endl;
        std::cout << "Invalid entries removed: " << invalidCount << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during repair: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error during repair" << std::endl;
    }
}