#include "ClientConnection.h"
#include <iostream>

using boost::asio::ip::tcp;

ClientConnection::ClientConnection()
    : _io(),
    _socket(_io){

    std::string address;
    uint16_t port = 0;

    loadServerInfo(address, port);

    if (address.empty() || port == 0) {
        throw std::runtime_error("Failed to load server.info.txt.");
    }
    std::cout << "Trying to connect to: " << address << ":" << port << "\n";

    try {
        tcp::resolver resolver(_io);
        auto endpoints = resolver.resolve(address, std::to_string(port));
        boost::asio::connect(_socket, endpoints);
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to connect to server: ") + e.what());
    }

    std::cout << "Connected to server\n";
}

ClientConnection::~ClientConnection() {
    if (_socket.is_open()) {
        boost::system::error_code ec;
        _socket.shutdown(tcp::socket::shutdown_both, ec);
        _socket.close(ec);
    }
}

std::vector<uint8_t> ClientConnection::sendAndReceive(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(_connMutex);

    sendData(data);
    auto buffer = receiveExact(HEADER_SIZE);
    auto payload = receiveExact(getPayloadSize(buffer));
    buffer.insert(buffer.end(), payload.begin(), payload.end());

    return buffer;
}

void ClientConnection::sendData(const std::vector<uint8_t>& data) {
    boost::asio::write(_socket, boost::asio::buffer(data, data.size()));
}


std::vector<uint8_t> ClientConnection::receiveExact(size_t size) {
    if (size == 0) {
        return {};
    }
    std::vector<uint8_t> buffer(size);

    boost::asio::read(_socket, boost::asio::buffer(buffer.data(), size));

    return buffer;
}

size_t ClientConnection::getPayloadSize(const std::vector<uint8_t>& data) const {
    if (data.size() < HEADER_SIZE) {
        throw std::runtime_error("getPayloadSize: response header too small");
    }

    return static_cast<size_t>(readUint32LE(data, PAYLOAD_OFFSET));
}

