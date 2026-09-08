#include "FileManager.h"

#include "Base64Wrapper.h"
#include "Constants.h"
#include "Utilities.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace {
    const std::string MY_INFO = "my.info.txt";
    const std::string SERVER_INFO = "server.info.txt";

    /**
     * Validates and parses client data from an open file.
     * Extracts the username, client ID, and private key, validates their format,
     * and updates the client context if valid.
     *
     * @param Receives an open file stream and the client context to populate
     * @return Returns true if the file content is valid and successfully loaded,
     * otherwise false
     */
    bool validateAndLoadMyInfo(std::ifstream& file, ClientContext& ctx);

    /**
     * Validates and parses server connection data from an open file.
     * Ensures the format is correct (address:port) and verifies both values.
     *
     * @param Receives an open file stream and references for address and port
     * @return Returns true if the file content is valid and successfully parsed,
     * otherwise false
     */
    bool validateAndLoadServerInfo(std::ifstream& file, std::string& address, uint16_t& port);
    
    /**
     * Retrieves the directory of the running executable.
     * Used to locate configuration files relative to the executable path.
     *
     * @return Returns the filesystem path of the executable's directory
     * @throws std::runtime_error If the executable path cannot be determined
     */
    std::filesystem::path getExecutableDirectory();
}

void loadMyInfo(ClientContext& ctx) {
    std::ifstream file(getExecutableDirectory() / MY_INFO);

    if (!file.is_open()) {
        std::cout << "my.info file was not found, please register (110) before any use.\n";
        return;
    }

    if (!validateAndLoadMyInfo(file, ctx)) {
        std::cout << "my.info is invalid, please use register (110).\n";
        return;
    }
}


void saveMyInfo(const ClientContext& ctx) {
    if (!ctx.hasClientID() || ctx.getName().empty() || ctx.getPrivateKey().empty()) {
        std::cout << "Cannot save my.info: client data is incomplete.\n";
        return;
    }

    std::ofstream file(getExecutableDirectory() / MY_INFO, std::ios::trunc);
    if (!file.is_open()) {
        std::cout << "Failed to create my.info file.\n";
        return;
    }

    std::string privateKeyBase64 = Base64Wrapper::encode(ctx.getPrivateKey());

    privateKeyBase64.erase(std::remove(privateKeyBase64.begin(), privateKeyBase64.end(), '\n'),
        privateKeyBase64.end());

    privateKeyBase64.erase(std::remove(privateKeyBase64.begin(), privateKeyBase64.end(), '\r'), 
        privateKeyBase64.end());

    file << ctx.getName() << '\n';
    file << bytesToHex(ctx.getClientID()) << '\n';
    file << privateKeyBase64;

    if (!file) {
        std::cout << "Failed to write my.info file.\n";
    }
}

void loadServerInfo(std::string& address, uint16_t& port) {
    address.clear();
    port = 0;

    std::ifstream file(getExecutableDirectory() / SERVER_INFO);

    if (!file.is_open()) {
        std::cout << "server.info.txt file was not found.\n";
        return;
    }

    if (!validateAndLoadServerInfo(file, address, port)) {
        std::cout << "server.info.txt is invalid.\n";
    }
}



namespace {

    bool validateAndLoadMyInfo(std::ifstream& file, ClientContext& ctx) {
        std::string name;
        std::string clientIdHex;
        std::string privateKeyBase64;

        if (!std::getline(file, name) || !std::getline(file, clientIdHex) ||
            !std::getline(file, privateKeyBase64)) {
            return false;
        }

        if (!isValidUsername(name)) { return false; }
        if (clientIdHex.size() != UUID_HEX_LENGTH) { return false; }

        for (unsigned char c : clientIdHex) {
            if (!std::isxdigit(c)) {
                return false;
            }
        }

        if (privateKeyBase64.empty()) { return false; }

        std::string extraLine;
        if (std::getline(file, extraLine)) { return false; }

        try {
            auto clientIdVec = uuidHex32ToBytes16(clientIdHex);

            std::array<uint8_t, CLIENT_ID_SIZE> clientId{};
            std::copy(clientIdVec.begin(), clientIdVec.end(), clientId.begin());

            std::string privateKeyRaw = Base64Wrapper::decode(privateKeyBase64);

            if (privateKeyRaw.empty()) {
                return false;
            }

            ctx.setName(name);
            ctx.setClientID(clientId);
            ctx.setPrivateKey(privateKeyRaw);
        }
        catch (...) {
            return false;
        }

        std::cout << "Welcome back " << name << "\n";
        return true;
    }

    bool validateAndLoadServerInfo(std::ifstream& file, std::string& address, uint16_t& port) {
        std::string line;

        if (!std::getline(file, line)) { return false; }
        if (file.peek() != EOF) { return false; }

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) { return false; }
        if (line.find(':', colonPos + 1) != std::string::npos) { return false; }

        std::string addressPart = line.substr(0, colonPos);
        std::string portPart = line.substr(colonPos + 1);

        if (addressPart.empty() || portPart.empty()) { return false; }

        for (unsigned char c : portPart) {
            if (!std::isdigit(c)) { return false; }
        }

        int parsedPort;
        try { parsedPort = std::stoi(portPart); }
        catch (...) { return false; }

        if (parsedPort < 1 || parsedPort > 65535) { return false; }

        int dots = 0;
        size_t start = 0;

        while (true) {
            size_t end = addressPart.find('.', start);
            std::string part = addressPart.substr(start, end - start);

            if (part.empty() || part.size() > 3) { return false; }

            for (unsigned char c : part) {
                if (!std::isdigit(c)) {
                    return false;
                }
            }

            int value;
            try {
                value = std::stoi(part);
            }
            catch (...) { return false; }

            if (value < 0 || value > 255) { return false; }

            if (end == std::string::npos) {
                break;
            }

            dots++;
            start = end + 1;
        }

        if (dots != 3) { return false; }

        address = addressPart;
        port = static_cast<uint16_t>(parsedPort);
        return true;
    }

    std::filesystem::path getExecutableDirectory() {
        char buffer[MAX_PATH];
        DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);

        if (length == 0 || length == MAX_PATH) {
            throw std::runtime_error("Failed to get executable path.");
        }

        return std::filesystem::path(buffer).parent_path();
    }
}
