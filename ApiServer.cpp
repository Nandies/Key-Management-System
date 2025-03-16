#include "ApiServer.h"
#include "Key.h"
#include <iostream>
#include <sstream>
#include <mutex>
#include <map>
#include <fstream>
#include <thread>
#include <chrono>

// Thread safety for concurrent API requests
std::mutex apiMutex;

// API key for authentication
const std::string API_KEY = "your-secret-api-key";

ApiServer::ApiServer() :
    keyManager(std::make_unique<KeyManager>()),
    running(false),
    port(8080),
    useHttps(false),
    certFile("server.crt"),
    keyFile("server.key") {
}

ApiServer::~ApiServer() {
    if (isRunning()) {
        stop();
    }
}

void ApiServer::start(int serverPort, bool withHttps, const std::string& sslCertFile, const std::string& sslKeyFile) {
    port = serverPort;
    useHttps = withHttps;

    if (!sslCertFile.empty()) {
        certFile = sslCertFile;
    }

    if (!sslKeyFile.empty()) {
        keyFile = sslKeyFile;
    }

    // If already running, return
    if (isRunning()) {
        std::cout << "Server already running on port " << port << std::endl;
        return;
    }

    // Start server thread
    running = true;
    serverThread = std::thread(&ApiServer::runServer, this);

    std::cout << "API server started on " << (useHttps ? "https" : "http") << "://localhost:" << port << std::endl;
}

void ApiServer::stop() {
    if (!isRunning()) {
        return;
    }

    std::cout << "Stopping API server..." << std::endl;
    running = false;

    // Clean up server thread
    if (serverThread.joinable()) {
        serverThread.join();
    }

    std::cout << "API server stopped" << std::endl;
}

bool ApiServer::isRunning() const {
    return running;
}

void ApiServer::runServer() {
    try {
        // Create a Crow app
        crow::SimpleApp app;

        // Setup routes
        setupRoutes(app);

        // Start the server
        std::cout << "Starting web server on port " << port << "..." << std::endl;

        // Configure app to listen on specified port
        app.port(port);

        // Start in non-blocking mode to allow graceful shutdown
        app.run_async();

        // Keep server running until stopped
        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // Shut down the server
        app.stop();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in server thread: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Unknown error in server thread" << std::endl;
    }
}

void ApiServer::setupRoutes(crow::SimpleApp& app) {
    // Health check endpoint (no authentication required)
    CROW_ROUTE(app, "/health")
        ([]() {
        return crow::response(200, "API server is running");
            });

    // Version endpoint (no authentication required)
    CROW_ROUTE(app, "/version")
        ([]() {
        std::string json = R"({
            "version": "1.0.0",
            "name": "Key Management System API",
            "description": "REST API for managing license keys"
        })";
        return crow::response(200, json);
            });

    // Authentication middleware function
    auto authenticateRequest = [](const crow::request& req) -> bool {
        auto apiKey = req.get_header_value("X-API-Key");
        return apiKey == API_KEY;
        };

    // Get all keys
    CROW_ROUTE(app, "/api/keys")
        ([this, authenticateRequest](const crow::request& req) {
        // Check authentication
        if (!authenticateRequest(req)) {
            return crow::response(401, R"({"error":"Unauthorized"})");
        }

        try {
            std::lock_guard<std::mutex> lock(apiMutex);
            auto keys = getAllKeys();

            std::stringstream json;
            json << R"({"keys":[)";

            for (size_t i = 0; i < keys.size(); i++) {
                if (i > 0) json << ",";
                json << R"({)";
                json << R"("id":)" << i << R"(,)";
                json << R"("value":")" << keys[i].getKeyValue() << R"(",)";
                json << R"("type":)" << static_cast<int>(keys[i].getKeyType()) << R"(,)";
                json << R"("typeName":")" << keys[i].getKeyTypeName() << R"(",)";
                json << R"("used":)" << (keys[i].getIsUsed() ? "true" : "false") << R"(,)";
                json << R"("discordUsername":")" << keys[i].getDiscordUsername() << R"(")";
                json << R"(})";
            }

            json << R"(]})";
            return crow::response(200, json.str());
        }
        catch (const std::exception& e) {
            return crow::response(500, R"({"error":"Internal server error: )" + std::string(e.what()) + R"("})");
        }
            });

    // Get keys by type
    CROW_ROUTE(app, "/api/keys/type/<int>")
        ([this, authenticateRequest](const crow::request& req, int typeInt) {
        // Check authentication
        if (!authenticateRequest(req)) {
            return crow::response(401, R"({"error":"Unauthorized"})");
        }

        try {
            std::lock_guard<std::mutex> lock(apiMutex);

            // Validate type parameter
            if (typeInt < 0 || typeInt > 3) {
                return crow::response(400, R"({"error":"Invalid key type. Must be 0-3"})");
            }

            KeyType keyType = static_cast<KeyType>(typeInt);

            // Filter keys by type
            auto allKeys = getAllKeys();
            std::vector<Key> filteredKeys;

            for (const auto& key : allKeys) {
                if (key.getKeyType() == keyType) {
                    filteredKeys.push_back(key);
                }
            }

            // Build response
            std::stringstream json;
            json << R"({"keys":[)";

            for (size_t i = 0; i < filteredKeys.size(); i++) {
                if (i > 0) json << ",";
                json << R"({)";
                json << R"("id":)" << i << R"(,)";
                json << R"("value":")" << filteredKeys[i].getKeyValue() << R"(",)";
                json << R"("type":)" << static_cast<int>(filteredKeys[i].getKeyType()) << R"(,)";
                json << R"("typeName":")" << filteredKeys[i].getKeyTypeName() << R"(",)";
                json << R"("used":)" << (filteredKeys[i].getIsUsed() ? "true" : "false") << R"(,)";
                json << R"("discordUsername":")" << filteredKeys[i].getDiscordUsername() << R"(")";
                json << R"(})";
            }

            json << R"(]})";
            return crow::response(200, json.str());
        }
        catch (const std::exception& e) {
            return crow::response(500, R"({"error":"Internal server error: )" + std::string(e.what()) + R"("})");
        }
            });

    // Create a new key
    CROW_ROUTE(app, "/api/keys")
        .methods("POST"_method)
        ([this, authenticateRequest](const crow::request& req) {
        // Check authentication
        if (!authenticateRequest(req)) {
            return crow::response(401, R"({"error":"Unauthorized"})");
        }

        try {
            // Simple manual JSON parsing since Crow's JSON handling varies by version
            std::string body = req.body;

            // Extract key value (very basic parsing, for demonstration only)
            size_t valuePos = body.find("\"value\"");
            if (valuePos == std::string::npos) {
                return crow::response(400, R"({"error":"Missing 'value' parameter"})");
            }

            valuePos = body.find(':', valuePos) + 1;
            // Skip whitespace
            while (valuePos < body.size() && (body[valuePos] == ' ' || body[valuePos] == '\t')) valuePos++;

            // Check if it's a string
            if (body[valuePos] != '"') {
                return crow::response(400, R"({"error":"'value' must be a string"})");
            }

            valuePos++; // Skip the opening quote
            size_t valueEnd = body.find('"', valuePos);
            if (valueEnd == std::string::npos) {
                return crow::response(400, R"({"error":"Invalid JSON format"})");
            }

            std::string keyValue = body.substr(valuePos, valueEnd - valuePos);
            if (keyValue.empty()) {
                return crow::response(400, R"({"error":"'value' cannot be empty"})");
            }

            // Extract key type
            size_t typePos = body.find("\"type\"");
            if (typePos == std::string::npos) {
                return crow::response(400, R"({"error":"Missing 'type' parameter"})");
            }

            typePos = body.find(':', typePos) + 1;
            // Skip whitespace
            while (typePos < body.size() && (body[typePos] == ' ' || body[typePos] == '\t')) typePos++;

            // Read type (assume it's a single digit 0-3)
            if (typePos >= body.size() || !std::isdigit(body[typePos])) {
                return crow::response(400, R"({"error":"'type' must be a number"})");
            }

            int keyTypeInt = body[typePos] - '0';
            if (keyTypeInt < 0 || keyTypeInt > 3) {
                return crow::response(400, R"({"error":"Invalid key type. Must be 0-3"})");
            }

            // Add the key
            std::lock_guard<std::mutex> lock(apiMutex);
            if (addKey(keyValue, static_cast<KeyType>(keyTypeInt))) {
                return crow::response(201, R"({"status":"success"})");
            }
            else {
                return crow::response(409, R"({"error":"Key already exists or couldn't be added"})");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, R"({"error":"Internal server error: )" + std::string(e.what()) + R"("})");
        }
            });

    // Mark key as used
    CROW_ROUTE(app, "/api/keys/<int>/use")
        .methods("PUT"_method)
        ([this, authenticateRequest](const crow::request& req, int keyId) {
        // Check authentication
        if (!authenticateRequest(req)) {
            return crow::response(401, R"({"error":"Unauthorized"})");
        }

        try {
            // Simple manual JSON parsing
            std::string body = req.body;

            // Extract discord username
            size_t usernamePos = body.find("\"discordUsername\"");
            if (usernamePos == std::string::npos) {
                return crow::response(400, R"({"error":"Missing 'discordUsername' parameter"})");
            }

            usernamePos = body.find(':', usernamePos) + 1;
            // Skip whitespace
            while (usernamePos < body.size() && (body[usernamePos] == ' ' || body[usernamePos] == '\t')) usernamePos++;

            // Check if it's a string
            if (body[usernamePos] != '"') {
                return crow::response(400, R"({"error":"'discordUsername' must be a string"})");
            }

            usernamePos++; // Skip the opening quote
            size_t usernameEnd = body.find('"', usernamePos);
            if (usernameEnd == std::string::npos) {
                return crow::response(400, R"({"error":"Invalid JSON format"})");
            }

            std::string discordUsername = body.substr(usernamePos, usernameEnd - usernamePos);

            // Mark key as used
            std::lock_guard<std::mutex> lock(apiMutex);
            if (markKeyAsUsed(keyId, discordUsername)) {
                return crow::response(200, R"({"status":"success"})");
            }
            else {
                return crow::response(404, R"({"error":"Key not found or already used"})");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, R"({"error":"Internal server error: )" + std::string(e.what()) + R"("})");
        }
            });

    // Mark key as unused
    CROW_ROUTE(app, "/api/keys/<int>/unuse")
        .methods("PUT"_method)
        ([this, authenticateRequest](const crow::request& req, int keyId) {
        // Check authentication
        if (!authenticateRequest(req)) {
            return crow::response(401, R"({"error":"Unauthorized"})");
        }

        try {
            // Mark key as unused
            std::lock_guard<std::mutex> lock(apiMutex);
            if (markKeyAsUnused(keyId)) {
                return crow::response(200, R"({"status":"success"})");
            }
            else {
                return crow::response(404, R"({"error":"Key not found or already unused"})");
            }
        }
        catch (const std::exception& e) {
            return crow::response(500, R"({"error":"Internal server error: )" + std::string(e.what()) + R"("})");
        }
            });

    // Get key statistics
    CROW_ROUTE(app, "/api/stats")
        ([this, authenticateRequest](const crow::request& req) {
        // Check authentication
        if (!authenticateRequest(req)) {
            return crow::response(401, R"({"error":"Unauthorized"})");
        }

        try {
            std::lock_guard<std::mutex> lock(apiMutex);
            auto statsJsonStr = getStatsJson();
            return crow::response(200, statsJsonStr);
        }
        catch (const std::exception& e) {
            return crow::response(500, R"({"error":"Internal server error: )" + std::string(e.what()) + R"("})");
        }
            });
}

// Implement helper methods that interface with KeyManager
std::vector<Key> ApiServer::getAllKeys() {
    return keyManager->getAllKeys();
}

bool ApiServer::addKey(const std::string& value, KeyType type) {
    try {
        // Create a Key object with the specified value and type
        Key key(value, type);

        // Use your existing method to add a key
        keyManager->addKey(key);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool ApiServer::markKeyAsUsed(int keyId, const std::string& discordUsername) {
    try {
        auto keys = keyManager->getAllKeys();

        // Check if key id is valid
        if (keyId < 0 || keyId >= static_cast<int>(keys.size())) {
            return false;
        }

        // Get the key value
        std::string keyValue = keys[keyId].getKeyValue();

        // Mark key as used by value
        return keyManager->markKeyByValue(keyValue, discordUsername);
    }
    catch (...) {
        return false;
    }
}

bool ApiServer::markKeyAsUnused(int keyId) {
    try {
        auto keys = keyManager->getAllKeys();

        // Check if key id is valid
        if (keyId < 0 || keyId >= static_cast<int>(keys.size())) {
            return false;
        }

        // Get the key value
        std::string keyValue = keys[keyId].getKeyValue();

        // Mark key as unused by value
        return keyManager->markKeyAsUnusedByValue(keyValue);
    }
    catch (...) {
        return false;
    }
}

std::string ApiServer::getStatsJson() {
    auto keys = getAllKeys();

    // Count totals, used, and by type
    int totalKeys = keys.size();
    int usedKeys = 0;

    std::map<KeyType, int> totalByType;
    std::map<KeyType, int> usedByType;

    for (const auto& key : keys) {
        if (key.getIsUsed()) {
            usedKeys++;
            usedByType[key.getKeyType()]++;
        }
        totalByType[key.getKeyType()]++;
    }

    std::stringstream json;
    json << R"({)";
    json << R"("totalKeys":)" << totalKeys << R"(,)";
    json << R"("usedKeys":)" << usedKeys << R"(,)";
    json << R"("availableKeys":)" << (totalKeys - usedKeys) << R"(,)";

    json << R"("keysByType":{)";

    bool first = true;
    for (int i = 0; i <= 3; i++) {
        KeyType type = static_cast<KeyType>(i);
        std::string typeName;

        switch (type) {
        case KeyType::Day: typeName = "Daily"; break;
        case KeyType::Week: typeName = "Weekly"; break;
        case KeyType::Month: typeName = "Monthly"; break;
        case KeyType::Lifetime: typeName = "Lifetime"; break;
        }

        if (!first) json << R"(,)";
        first = false;

        json << R"(")" << typeName << R"(":{)";
        json << R"("total":)" << totalByType[type] << R"(,)";
        json << R"("used":)" << usedByType[type] << R"(,)";
        json << R"("available":)" << (totalByType[type] - usedByType[type]);
        json << R"(})";
    }

    json << R"(})";
    json << R"(})";

    return json.str();
}