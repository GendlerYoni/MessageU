#include "Utilities.h"

#include <cctype>
#include <stdexcept>

namespace {
    /**
     * Converts a single hexadecimal character into its numeric value.
     *
     * @param Receives a lowercase hexadecimal character
     * @return Returns the numeric value of the given hexadecimal digit
     */
    uint8_t hexNibble(char c);
}

std::string toLowerAscii(std::string s) {
	for (char& c : s)
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	return s;
}

bool isValidHexUUID(const std::string& s) {
	if (s.length() != UUID_HEX_LENGTH)
		return false;

	for (char c : s) {
		if (!std::isxdigit(static_cast<unsigned char>(c)))
			return false;
	}

	return true;
}

std::vector<uint8_t> uuidHex32ToBytes16(const std::string& hex32) {
    if (!isValidHexUUID(hex32)) {
        throw std::logic_error("uuidHex32ToBytes16: invalid UUID hex string");
    }
    std::string lowered = toLowerAscii(hex32);

    std::vector<uint8_t> out(CLIENT_ID_SIZE); 

    for (size_t i = 0; i < CLIENT_ID_SIZE; ++i) {
        uint8_t hi = hexNibble(lowered[2 * i]);
        uint8_t lo = hexNibble(lowered[2 * i + 1]);
        out[i] = static_cast<uint8_t>((hi << 4) | lo);
    }

    return out;
}

std::string bytesToHex(const std::array<uint8_t, CLIENT_ID_SIZE>& bytes)
{
    static const char* HEX = "0123456789abcdef";
    std::string ret;
    ret.resize(UUID_HEX_LENGTH); 
    for (size_t i = 0; i < CLIENT_ID_SIZE; ++i) {
        uint8_t b = bytes[i];
        ret[2 * i] = HEX[(b >> 4) & 0x0F];
        ret[2 * i + 1] = HEX[b & 0x0F];
    }
    return ret;
}

void appendUint16LE(std::vector<uint8_t>& buf, uint16_t value) {
    buf.push_back(static_cast<uint8_t>(value & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void appendUint32LE(std::vector<uint8_t>& buf, uint32_t value) {
    buf.push_back(static_cast<uint8_t>(value & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buf.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

uint32_t readUint32LE(const std::vector<uint8_t>& data, size_t offset) {

    if (data.size() < offset + UINT32_SIZE)
        throw std::runtime_error("readUint32LE: not enough bytes");

    uint32_t value = 0;

    value |= static_cast<uint32_t>(data[offset]);
    value |= static_cast<uint32_t>(data[offset + 1]) << 8;
    value |= static_cast<uint32_t>(data[offset + 2]) << 16;
    value |= static_cast<uint32_t>(data[offset + 3]) << 24;

    return value;
}


bool isValidUsername(const std::string& name) {
    if (name.empty() || name.size() >= NAME_SIZE) {
        return false;
    }

    for (unsigned char c : name) {
        if (c > 127) {
            return false;
        }
    }

    return true;
}

namespace {
    uint8_t hexNibble(char c) {
        return (c <= '9')
            ? static_cast<uint8_t>(c - '0')
            : static_cast<uint8_t>(10 + (c - 'a'));
    }
}