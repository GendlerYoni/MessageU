/**
 * Represents a protocol message sent from the client to the server.
 *
 * This module stores the message header fields and optional payload,
 * and provides functionality for building a serialized binary buffer
 * according to the MessageU protocol.
 *
 * It is used to construct protocol-compliant requests by combining
 * the client ID, version, request code, payload size, and payload data
 * in the correct order and binary format.
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "Constants.h"
#include "Utilities.h"

constexpr uint8_t CLIENT_VERSION = 1;

class Message {

protected:
	std::array<uint8_t, CLIENT_ID_SIZE> _clientID;
	uint8_t _version = CLIENT_VERSION;
	uint16_t _code;
	std::vector<uint8_t> _payload;

public:
	/**
	 * Creates a message with no payload.
	 * Initializes the message header fields using the given client ID and request code.
	 *
	 * @param Receives the client ID and the request code for the message
	 */
	Message(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID, uint16_t code);

	/**
	 * Creates a message with a payload.
	 * Initializes the message header fields and stores the provided payload data.
	 *
	 * @param Receives the client ID, the request code, and the payload data for the message
	 */
	Message(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID, uint16_t code,
		const std::vector<uint8_t>& payload);

	/**
	 * Builds a serialized binary buffer representing the full message.
	 * Appends the client ID, version, request code, payload size, and payload
	 * in the correct order according to the protocol format.
	 *
	 * @return Returns a serialized buffer containing the complete message
	 */
	std::vector<uint8_t> createBuffer() const;
};
