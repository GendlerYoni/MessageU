/**
 * Defines all request, response, and message type codes used by the client
 * according to the MessageU protocol.
 *
 * Includes user menu actions, protocol request codes sent to the server,
 * response codes received from the server, and message type identifiers
 * used within message payloads.
 *
 * Centralizing these values ensures consistency, readability, and prevents
 * the use of hardcoded numeric values throughout the implementation.
 */

#pragma once

#include <cstdint>

namespace Code {

	constexpr uint16_t REGISTER = 110;
	constexpr uint16_t CLIENTS_LIST = 120;
	constexpr uint16_t PUBLIC_KEY = 130;
	constexpr uint16_t WAITING_MESSAGES = 140;
	constexpr uint16_t SEND_TEXT = 150;
	constexpr uint16_t REQUEST_SYMMETRIC_KEY = 151;
	constexpr uint16_t SEND_SYMMETRIC_KEY = 152;
	constexpr uint16_t EXIT = 0;

	constexpr uint16_t REQUEST_REGISTER = 700;
	constexpr uint16_t REQUEST_CLIENTS_LIST = 701;
	constexpr uint16_t REQUEST_PUBLIC_KEY = 702;

	constexpr uint16_t REQUEST_ALL_MESSAGES = 703;

	constexpr uint16_t REQUEST_SEND_TEXT = 703;
	constexpr uint16_t REQUEST_REQUEST_SYMMETRIC_KEY = 703;
	constexpr uint16_t REQUEST_SEND_SYMMETRIC_KEY = 703;
	constexpr uint16_t REQUEST_WAITING_MESSAGES = 704;

	constexpr uint16_t RESPONSE_REGISTER = 2100;
	constexpr uint16_t RESPONSE_CLIENT_LIST = 2101;
	constexpr uint16_t RESPONSE_PUBLIC_KEY = 2102;
	constexpr uint16_t RESPONSE_MESSAGE_SENT = 2103;
	constexpr uint16_t RESPONSE_WAITING_MESSAGES = 2104;
	constexpr uint16_t ERROR_GENERAL = 9000;
}

namespace MessageType {
	constexpr uint8_t REQUEST_SYMMETRIC_KEY = 1;
	constexpr uint8_t SEND_SYMMETRIC_KEY = 2;
	constexpr uint8_t SEND_TEXT = 3;
}