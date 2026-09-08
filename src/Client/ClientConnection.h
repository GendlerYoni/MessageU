/**
 * Handles all client-server communication using TCP sockets.
 *
 * This module is responsible for establishing and managing the connection
 * to the server, sending serialized requests, and receiving protocol-compliant
 * responses.
 *
 * It ensures thread-safe communication using a mutex, performs blocking
 * reads and writes over the socket, and handles message boundaries by
 * reading the response header and payload separately according to the protocol.
 */

#pragma once
#include "FileManager.h"
#include "Constants.h"
#include "Utilities.h"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>
#include <boost/asio.hpp>

class ClientConnection {
public:
    /**
     * Establishes a connection to the server.
     * Loads the server address and port from configuration,
     * resolves the endpoint, and connects the TCP socket.
     *
     * @throws std::runtime_error If the server information cannot be loaded
     * or if the connection to the server fails
     */
    ClientConnection();
    /**
     * Closes the connection to the server and releases socket resources.
     * Ensures the socket is gracefully shut down and closed if it is still open.
     */
    ~ClientConnection();

    // Disable copying to prevent multiple objects managing the same socket
    ClientConnection(const ClientConnection&) = delete;
    ClientConnection& operator=(const ClientConnection&) = delete;

    /**
     * Sends a request to the server and receives the corresponding response.
     * Ensures thread-safe access to the connection, sends the serialized data,
     * reads the response header and payload, and returns the complete message buffer.
     *
     * @param Receives a serialized request buffer to be sent to the server
     * @return Returns a complete response buffer including both header and payload
     */
    std::vector<uint8_t> sendAndReceive(const std::vector<uint8_t>& data);

private:
    boost::asio::io_context _io;
    boost::asio::ip::tcp::socket _socket;
    std::mutex _connMutex;

    /**
     * Sends raw serialized data to the server over the socket.
     *
     * @param Receives a serialized buffer to be sent to the server
     */
    void sendData(const std::vector<uint8_t>& data);

    /**
     * Receives an exact number of bytes from the server.
     * Performs a blocking read until the requested size is fully received.
     *
     * @param Receives the number of bytes to read from the socket
     * @return Returns a buffer containing the received data
     */
    std::vector<uint8_t> receiveExact(size_t size);

    /**
     * Extracts the payload size from a response header.
     * Reads the payload size field according to the protocol format.
     *
     * @param Receives a buffer containing at least the response header
     * @return Returns the payload size extracted from the header
     * @throws std::runtime_error If the buffer is smaller than the expected header size
     */
    size_t getPayloadSize(const std::vector<uint8_t>& data) const;
};
