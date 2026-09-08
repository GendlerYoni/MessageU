#include "ClientContext.h"

void ClientContext::setClientID(const std::array<uint8_t, CLIENT_ID_SIZE>& id) { _clientID = id; }
const std::array<uint8_t, CLIENT_ID_SIZE>& ClientContext::getClientID() const { return _clientID; }

bool ClientContext::hasClientID() const {
    for (uint8_t byte : _clientID) {
        if (byte != 0) {
            return true;
        }
    }
    return false;
}

void ClientContext::setName(const std::string& name) { _userName = name; }
const std::string& ClientContext::getName() const { return _userName; }

void ClientContext::setPrivateKey(const std::string& privKey) { _privateKey = privKey; }
const std::string& ClientContext::getPrivateKey() const { return _privateKey; }

void ClientContext::setFriendBase(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid, const std::string& friendName){
    std::lock_guard<std::mutex> lock(_friendsMutex);

    Friend& f = _friends[uuid];
    f.friendID = uuid;
    f.friendName = friendName;
}


std::optional<ClientContext::Friend> ClientContext::tryGetFriendCopy(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid) const{
    std::lock_guard<std::mutex> lock(_friendsMutex);

    auto it = _friends.find(uuid);
    if (it == _friends.end()) return std::nullopt;

    return it->second; 
}

std::optional<std::array<uint8_t, CLIENT_ID_SIZE>> ClientContext::tryGetFriendIdByName(const std::string& targetName) const{
    std::lock_guard<std::mutex> lock(_friendsMutex);

    for (const auto& kv : _friends) {
        if (kv.second.friendName == targetName) {
            return kv.second.friendID;
        }
    }
    return std::nullopt;
}

void ClientContext::setFriendPublicKey(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid,
    const std::array<uint8_t, PUBLIC_KEY_SIZE>& pubKey) {
    std::lock_guard<std::mutex> lock(_friendsMutex);

    auto it = _friends.find(uuid);
    if (it == _friends.end()) return;
    it->second.publicKey = pubKey;
}

void ClientContext::setFriendSymmetricKey(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid, const std::array<uint8_t, SYMMETRIC_KEY_SIZE>& symKey){
    std::lock_guard<std::mutex> lock(_friendsMutex);

    auto it = _friends.find(uuid);
    if (it == _friends.end()) return;
    it->second.symmetricKey = symKey;
}

bool ClientContext::canSendText(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid) const{
    std::lock_guard<std::mutex> lock(_friendsMutex);

    auto it = _friends.find(uuid);
    if (it == _friends.end()) return false;
    return it->second.symmetricKey.has_value();
}

bool ClientContext::canSendSymmetricKey(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid) const{
    std::lock_guard<std::mutex> lock(_friendsMutex);

    auto it = _friends.find(uuid);
    if (it == _friends.end()) return false;
    return it->second.publicKey.has_value();
}
