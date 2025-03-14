#include "UserInterface.h"
#include <iostream>
#include <string>

UserInterface::UserInterface(KeyManager& manager) : keyManager(manager) {}

void UserInterface::displayMainMenu() {
    std::cout << "\nMAIN MENU:" << std::endl;
    std::cout << "1. Import keys from text file" << std::endl;
    std::cout << "2. Display all keys" << std::endl;
    std::cout << "3. Display keys by type" << std::endl;
    std::cout << "4. Mark key as used" << std::endl;
    std::cout << "5. Mark key as unused" << std::endl;
    std::cout << "6. Search by Discord username" << std::endl;
    std::cout << "7. Display key statistics" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Enter choice: ";
}

KeyType promptForKeyType() {
    std::cout << "Select key type:" << std::endl;
    std::cout << "1. Daily" << std::endl;
    std::cout << "2. Weekly" << std::endl;
    std::cout << "3. Monthly" << std::endl;
    std::cout << "4. Lifetime" << std::endl;
    std::cout << "Enter choice: ";

    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1: return KeyType::Day;
    case 2: return KeyType::Week;
    case 3: return KeyType::Month;
    case 4: return KeyType::Lifetime;
    default:
        std::cout << "Invalid choice. Defaulting to Daily." << std::endl;
        return KeyType::Day;
    }
}

void UserInterface::run() {
    std::cout << "========================================" << std::endl;
    std::cout << "         PEEPO KEY MANAGER          " << std::endl;
    std::cout << "========================================" << std::endl;

    int choice;
    bool running = true;

    while (running) {
        displayMainMenu();
        std::cin >> choice;

        switch (choice) {
        case 1: {
            std::cin.ignore(); // Clear the newline character
            std::string filename;
            std::cout << "Enter the path to the text file: ";
            std::getline(std::cin, filename);

            KeyType type = promptForKeyType();
            keyManager.importKeysFromFile(filename, type);
            break;
        }
        case 2:
            keyManager.displayKeys();
            break;
        case 3: {
            KeyType type = promptForKeyType();
            keyManager.displayKeysByType(type);
            break;
        }
        case 4:
            keyManager.markKeyAsUsed();
            break;
        case 5:
            keyManager.markKeyAsUnused();
            break;
        case 6:
            keyManager.searchByDiscordUsername();
            break;
        case 7:
            keyManager.displayKeyStatistics();
            break;
        case 0:
            running = false;
            std::cout << "Exiting program. Goodbye!" << std::endl;
            break;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}