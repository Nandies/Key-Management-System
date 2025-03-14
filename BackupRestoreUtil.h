#ifndef BACKUP_RESTORE_UTIL_H
#define BACKUP_RESTORE_UTIL_H

#include <string>
#include <vector>

class BackupRestoreUtil {
public:
    // Backup the database to a file
    static bool backupDatabase(const std::string& filename);

    // Restore the database from a file
    static bool restoreDatabase(const std::string& filename);

    // Attempt to repair a corrupted database
    static void repairDatabase();
};

#endif // BACKUP_RESTORE_UTIL_H