/**
 * Defines all fixed protocol-related constants used throughout the client.
 *
 * Includes sizes of protocol fields such as client IDs, headers, message IDs,
 * payload offsets, and cryptographic key sizes.
 *
 * These constants ensure consistent parsing and construction of binary messages
 * according to the MessageU protocol and help avoid the use of magic numbers
 * across the codebase.
 */

#pragma once

constexpr size_t CLIENT_ID_SIZE = 16;
constexpr size_t UUID_HEX_LENGTH = CLIENT_ID_SIZE * 2;
constexpr size_t NAME_SIZE = 255;
constexpr size_t MSG_ID_SIZE = 4;
constexpr size_t HEADER_SIZE = 7;
constexpr size_t PUBLIC_KEY_SIZE = 160;
constexpr size_t TYPE_SIZE = 1;
constexpr size_t MSG_SIZE = 4;
constexpr size_t SYMMETRIC_KEY_SIZE = 16;
constexpr size_t PAYLOAD_OFFSET = 3;
constexpr size_t RESPONSE_CODE_OFFSET = 1;



