#include "Application.h"
#include "KeyManager.h"
#include "BackupRestoreUtil.h"
#include <iostream>
#include <string>

// Function to handle command-line arguments for batch operations
void processCommandLine(int argc, char* argv[]) {
    if (argc < 2) {
        return; // No command-line arguments, run in interactive mode
    }

    std::string command = argv[1];

    if (command == "import_file" && argc >= 4) {
        // import_file [filename] [key_type]
        try {
            std::string filename = argv[2];
            int keyTypeInt = std::stoi(argv[3]);
            KeyType keyType;

            switch (keyTypeInt) {
            case 1: keyType = KeyType::Day; break;
            case 2: keyType = KeyType::Week; break;
            case 3: keyType = KeyType::Month; break;
            case 4: keyType = KeyType::Lifetime; break;
            default: keyType = KeyType::Day;
            }

            KeyManager keyManager;
            keyManager.importKeysFromFile(filename, keyType);
            std::cout << "Import completed." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error during import: " << e.what() << std::endl;
        }
        return;
    }

    if (command == "backup_db" && argc >= 3) {
        // backup_db [backup_filename]
        try {
            std::string filename = argv[2];
            BackupRestoreUtil::backupDatabase(filename);
        }
        catch (const std::exception& e) {
            std::cerr << "Error during backup: " << e.what() << std::endl;
        }
        return;
    }

    if (command == "restore_db" && argc >= 3) {
        // restore_db [backup_filename]
        try {
            std::string filename = argv[2];
            BackupRestoreUtil::restoreDatabase(filename);
        }
        catch (const std::exception& e) {
            std::cerr << "Error during restore: " << e.what() << std::endl;
        }
        return;
    }

    if (command == "repair_db") {
        // repair_db
        try {
            BackupRestoreUtil::repairDatabase();
        }
        catch (const std::exception& e) {
            std::cerr << "Error during repair: " << e.what() << std::endl;
        }
        return;
    }

    // Unknown command
    std::cerr << "Unknown command: " << command << std::endl;
    std::cout << "Supported commands:" << std::endl;
    std::cout << "  import_file [filename] [key_type]" << std::endl;
    std::cout << "  backup_db [backup_filename]" << std::endl;
    std::cout << "  restore_db [backup_filename]" << std::endl;
    std::cout << "  repair_db" << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        // Check for command-line arguments
        if (argc > 1) {
            processCommandLine(argc, argv);
            return 0;
        }

        // No command-line arguments, run in interactive mode
        Application app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred." << std::endl;
        return 1;
    }

    return 0;
}