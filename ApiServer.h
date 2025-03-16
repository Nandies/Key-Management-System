#ifndef API_SERVER_H
#define API_SERVER_H

#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include "KeyManager.h"
// Configure Crow to use Boost.ASIO
#define CROW_USE_BOOST_ASIO
#include <crow.h>

class ApiServer {
private:
    std::unique_ptr<KeyManager> keyManager;
    std::thread serverThread;
    std::atomic<bool> running;
    int port;
    bool useHttps;
    std::string certFile;
    std::string keyFile;

    // Helper methods to interface with KeyManager
    std::vector<Key> getAllKeys();
    bool addKey(const std::string& value, KeyType type);
    bool markKeyAsUsed(int keyId, const std::string& discordUsername);
    bool markKeyAsUnused(int keyId);
    std::string getStatsJson();

    // Internal server runner method
    void runServer();

    // Setup routes for the Crow app
    void setupRoutes(crow::SimpleApp& app);

public:
    ApiServer();
    ~ApiServer();

    // Start the server on the specified port
    void start(int serverPort = 8080, bool withHttps = false,
        const std::string& sslCertFile = "server.crt",
        const std::string& sslKeyFile = "server.key");

    // Stop the server
    void stop();

    // Check if server is running
    bool isRunning() const;
};

#endif // API_SERVER_H