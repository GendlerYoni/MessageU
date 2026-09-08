#include "Message.h"

Message::Message(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID, uint16_t code)
: _clientID(clientID), _code(code){
}

Message::Message(const std::array<uint8_t, CLIENT_ID_SIZE>& clientID, uint16_t code, const std::vector<uint8_t>& payload)
	: _clientID(clientID), _code(code), _payload(payload){
}

std::vector<uint8_t> Message::createBuffer() const {
    std::vector<uint8_t> buffer;
    buffer.insert(buffer.end(), _clientID.begin(), _clientID.end());

    buffer.push_back(_version);

    appendUint16LE(buffer, _code);
    appendUint32LE(buffer, static_cast<uint32_t>(_payload.size()));
    
    buffer.insert(buffer.end(), _payload.begin(), _payload.end());

    return buffer;
}




