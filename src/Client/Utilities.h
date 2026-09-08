/**
 * Provides helper functions used across the client implementation.
 *
 * This module contains utility functions for string handling, UUID validation
 * and conversion, username validation, hexadecimal encoding, and reading or
 * writing numeric values in little-endian format.
 *
 * These helpers are used to support protocol parsing, message construction,
 * input validation, and safe conversion between textual and binary data.
 */

#pragma once

#include "Constants.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

constexpr size_t UINT32_SIZE = 4;

/**
 * Converts an ASCII string to lowercase.
 *
 * @param Receives a string to convert
 * @return Returns the lowercase version of the input string
 */
std::string toLowerAscii(std::string s);

/**
 * Checks whether a string is a valid hexadecimal UUID representation.
 * Validates both the expected length and that all characters are hexadecimal digits.
 *
 * @param Receives a string to validate
 * @return Returns true if the string is a valid 32-character hexadecimal UUID, otherwise false
 */
bool isValidHexUUID(const std::string& s);

/**
 * Converts a hexadecimal UUID string into its binary byte representation.
 * Validates the input format and converts each hexadecimal pair into one byte.
 *
 * @param Receives a 32-character hexadecimal UUID string
 * @return Returns a byte vector containing the binary UUID representation
 * @throws std::logic_error If the input is not a valid hexadecimal UUID
 */
std::vector<uint8_t> uuidHex32ToBytes16(const std::string& hex32);

/**
 * Appends a 16-bit unsigned integer to a buffer in little-endian format.
 *
 * @param Receives a target buffer and a 16-bit value to append
 */
void appendUint16LE(std::vector<uint8_t>& buf, uint16_t value);

/**
 * Appends a 32-bit unsigned integer to a buffer in little-endian format.
 *
 * @param Receives a target buffer and a 32-bit value to append
 */
void appendUint32LE(std::vector<uint8_t>& buf, uint32_t value);

/**
 * Converts a binary client ID into a hexadecimal string representation.
 *
 * @param Receives a fixed-size byte array containing the client ID
 * @return Returns the hexadecimal string representation of the client ID
 */
std::string bytesToHex(const std::array<uint8_t, CLIENT_ID_SIZE>& bytes);

/**
 * Reads a 32-bit unsigned integer from a buffer in little-endian format.
 *
 * @param Receives a data buffer and an offset indicating where the value starts
 * @return Returns the extracted 32-bit value
 * @throws std::runtime_error If there are not enough bytes in the buffer
 */
uint32_t readUint32LE(const std::vector<uint8_t>& data, size_t offset);

/**
 * Checks whether a username is valid according to the client rules.
 * Ensures the name is not empty, fits within the allowed size,
 * and contains ASCII characters only.
 *
 * @param Receives a username string to validate
 * @return Returns true if the username is valid, otherwise false
 */
bool isValidUsername(const std::string& name);
