# 🔑 Key Management System

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/yourusername/key-management-system)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)]()

A modern, lightweight C++ application for managing software license keys with support for different subscription types.

## ✨ Features

- **Multiple Key Types**: Support for Daily, Weekly, Monthly, and Lifetime keys
- **Status Tracking**: Mark keys as used/unused and associate with Discord usernames
- **Search Functionality**: Quickly find keys by Discord username
- **Detailed Statistics**: View key usage and availability statistics
- **Import Utility**: Easily import keys from text files
- **Data Protection**: Backup and restore database functionality
- **Repair Tools**: Database repair for corrupted data files
- **Dual Usage Modes**: Interactive console menu and command-line batch operations

## 🚀 Getting Started

### Prerequisites

- Windows 10/11
- Visual Studio 2019 or newer with C++20 support
- Windows SDK 10.0 or newer

### Installation

1. Clone the repository:
```bash
git clone https://github.com/yourusername/key-management-system.git
cd key-management-system
```

2. Open the solution in Visual Studio:
```
Key-Management-System.sln
```

3. Build the solution (F7 or Build → Build Solution)

4. Run the executable (F5 or Debug → Start Debugging)

## 📖 Usage

### Interactive Mode

Launch the application without any command-line arguments to use the interactive console interface:

```bash
KeyManagementSystem.exe
```

This will present you with the main menu:

```
========================================
         NANDIES KEY MANAGER          
========================================

MAIN MENU:
1. Import keys from text file
2. Display all keys
3. Display keys by type
4. Mark key as used
5. Mark key as unused
6. Search by Discord username
7. Display key statistics
0. Exit
Enter choice:
```

### Command-Line Mode

For batch operations, the following commands are supported:

```bash
# Import keys from a file
KeyManagementSystem.exe import_file path/to/keys.txt 1

# Backup the database
KeyManagementSystem.exe backup_db path/to/backup.csv

# Restore from backup
KeyManagementSystem.exe restore_db path/to/backup.csv

# Repair database
KeyManagementSystem.exe repair_db
```

#### Key Types:
- 1: Daily
- 2: Weekly
- 3: Monthly
- 4: Lifetime

### Key Format

When importing keys from a text file, each key should be on a separate line:

```
KEY1-ABCD-EFGH-1234
KEY2-IJKL-MNOP-5678
KEY3-QRST-UVWX-9012
```

## 🔧 Database

Keys are stored in CSV format in:
```
%APPDATA%\KeyManager\keys.csv
```

Format:
```
keyValue|typeValue|isUsed|discordUsername
```

Example:
```
KEY1-ABCD-EFGH-1234|3|1|username#1234
KEY2-IJKL-MNOP-5678|4|0|
```

## 🛠️ Technical Details

### Technology Stack

- C++20
- Standard Library
- Windows API for file operations

### Project Structure

| Component | Description |
|-----------|-------------|
| `Key` | Individual license key with properties |
| `KeyCollection` | Collection manager for keys |
| `KeyManager` | Core business logic |
| `IKeyStorage` | Storage interface |
| `FileSystemStorage` | File-based storage implementation |
| `UserInterface` | Console UI management |
| `BackupRestoreUtil` | Database operations |

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🗺️ Roadmap

Our vision for the future development of the Key Management System focuses on two key areas:

### Discord Bot Integration

- **REST API Development**: Create a secure API layer to allow Discord bot integration
- **Bot Command Structure**: Design intuitive Discord commands for key management operations
- **Webhook Support**: Implement webhooks for real-time notifications (key usage, low inventory, etc.)
- **Multi-Server Support**: Allow the bot to manage keys across multiple Discord servers
- **Discord Role Integration**: Tie key types to specific Discord roles
- **Automated Distribution**: Set up automatic key distribution based on user actions or time triggers

### Enhanced Security

- **Key Encryption**: Implement AES-256 encryption for keys stored in the database
- **Secure Authentication**: Add robust authentication for API access
- **Request Validation**: Add signature verification for all bot commands
- **Audit Logging**: Comprehensive logging system for all key operations
- **Intrusion Detection**: Monitor and alert on suspicious access patterns
- **Rate Limiting**: Prevent abuse through intelligent rate limiting
- **Data Sanitization**: Improve input validation and sanitization

### Additional Improvements

- **Web Dashboard**: Create a web-based admin interface
- **Database Migration**: Support for SQL databases (MySQL, PostgreSQL)
- **Analytics**: Generate insights about key usage patterns
- **Bulk Operations**: Support for bulk key management operations
- **Localization**: Support for multiple languages
- **Cloud Backup**: Automatic cloud backups of key database

---

## 🙏 Acknowledgements

- [Visual Studio](https://visualstudio.microsoft.com/) - Development environment
- [Windows API](https://docs.microsoft.com/en-us/windows/win32/api/) - File system operations
- [Discord API](https://discord.com/developers/docs/intro) - For future bot integration

---

<p align="center">
  Made with ❤️ by Nandies
</p>
