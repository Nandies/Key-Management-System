#include "Application.h"
#include "KeyManager.h"
#include "BackupRestoreUtil.h"
#include "ApiServer.h"
#include <iostream>
#include <string>
#include <memory>
#include <csignal>
#include <crow.h>

// Global ApiServer instance to allow signal handling
std::unique_ptr<ApiServer> apiServer;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "Received signal " << signal << std::endl;
    if (apiServer && apiServer->isRunning()) {
        std::cout << "Stopping API server..." << std::endl;
        apiServer->stop();
    }
    exit(signal);
}

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

    if (command == "start_api") {
        // start_api [port] [use_https] [cert_file] [key_file]
        try {
            int port = 8080; // Default port
            bool useHttps = false;
            std::string certFile = "server.crt";
            std::string keyFile = "server.key";

            if (argc >= 3) {
                port = std::stoi(argv[2]);
            }

            if (argc >= 4) {
                useHttps = (std::string(argv[3]) == "true" || std::string(argv[3]) == "1");
            }

            if (argc >= 5) {
                certFile = argv[4];
            }

            if (argc >= 6) {
                keyFile = argv[5];
            }

            // Register signal handlers for graceful shutdown
            std::signal(SIGINT, signalHandler);
            std::signal(SIGTERM, signalHandler);

            // Create and start API server
            apiServer = std::make_unique<ApiServer>();
            apiServer->start(port, useHttps, certFile, keyFile);

            std::cout << "API server started. Press Ctrl+C to stop." << std::endl;

            // Keep the main thread alive until interrupted
            while (apiServer->isRunning()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error starting API server: " << e.what() << std::endl;
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
    std::cout << "  start_api [port=8080] [use_https=false] [cert_file=server.crt] [key_file=server.key]" << std::endl;
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