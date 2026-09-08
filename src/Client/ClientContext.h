/**
 * Stores and manages all client-related data during runtime.
 *
 * This module maintains the client's identity, cryptographic keys,
 * and a cache of known clients ("friends") including their IDs,
 * names, public keys, and symmetric keys.
 *
 * Provides thread-safe access and updates to shared data using a mutex,
 * and exposes helper methods for retrieving and validating client-related information.
 */

#pragma once

#include "Constants.h"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <optional>
#include <mutex>

class ClientContext {
public:
    struct Friend {
        std::array<uint8_t, CLIENT_ID_SIZE> friendID{};
        std::string friendName{};
        std::optional<std::array<uint8_t, PUBLIC_KEY_SIZE>> publicKey;
        std::optional<std::array<uint8_t, SYMMETRIC_KEY_SIZE>> symmetricKey;
    };

    /**
     * Checks whether the client has been assigned a valid client ID.
     * Determines if the stored client ID contains any non-zero value.
     *
     * @return Returns true if a valid client ID exists, otherwise false
     */
    bool hasClientID() const;

    // Getters and setters for managing client ID, username, and private key
    void setClientID(const std::array<uint8_t, CLIENT_ID_SIZE>& id);
    const std::array<uint8_t, CLIENT_ID_SIZE>& getClientID() const;

    void setName(const std::string& name);
    const std::string& getName() const;

    void setPrivateKey(const std::string& privKey);
    const std::string& getPrivateKey() const;

    /**
     * Adds or updates basic information for a client in the local cache.
     * Stores the client's ID and name, creating a new entry if it does not exist.
     * Ensures thread-safe access to the shared data.
     *
     * @param Receives a client ID and the corresponding username
     */
    void setFriendBase(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid, const std::string& name);

    /**
     * Retrieves a copy of a client's data from the local cache.
     * Performs a lookup by client ID and returns the result if found.
     * Ensures thread-safe access to the shared data.
     *
     * @param Receives a client ID to search for
     * @return Returns an optional containing a copy of the client data if found,
     * or std::nullopt if the client does not exist
     */
    std::optional<Friend> tryGetFriendCopy(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid) const;

    /**
     * Searches for a client ID by username in the local cache.
     * Iterates over stored clients and returns the matching ID if found.
     * Ensures thread-safe access to the shared data.
     *
     * @param Receives a target username to search for
     * @return Returns an optional containing the client ID if found,
     * or std::nullopt if no matching client exists
     */
    std::optional<std::array<uint8_t, CLIENT_ID_SIZE>> tryGetFriendIdByName(const std::string& targetName) const;

    /**
     * Stores a public key for a specific client in the local cache.
     * Updates the existing entry if the client is found.
     * Ensures thread-safe access to the shared data.
     *
     * @param Receives a client ID and the corresponding public key
     */
    void setFriendPublicKey(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid,
        const std::array<uint8_t, PUBLIC_KEY_SIZE>& pubKey);

    /**
     * Stores a symmetric key for a specific client in the local cache.
     * Updates the existing entry if the client is found.
     * Ensures thread-safe access to the shared data.
     *
     * @param Receives a client ID and the corresponding symmetric key
     */
    void setFriendSymmetricKey(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid,
        const std::array<uint8_t, SYMMETRIC_KEY_SIZE>& symKey);

    /**
     * Checks whether a text message can be sent to a specific client.
     * Verifies that the client exists and has a valid symmetric key.
     * Ensures thread-safe access to the shared data.
     *
     * @param Receives a client ID to check
     * @return Returns true if a symmetric key exists for the client, otherwise false
     */
    bool canSendText(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid) const;

    /**
     * Checks whether a symmetric key can be sent to a specific client.
     * Verifies that the client exists and has a valid public key.
     * Ensures thread-safe access to the shared data.
     *
     * @param Receives a client ID to check
     * @return Returns true if a public key exists for the client, otherwise false
     */
    bool canSendSymmetricKey(const std::array<uint8_t, CLIENT_ID_SIZE>& uuid) const;

private:
    std::array<uint8_t, CLIENT_ID_SIZE> _clientID{};
    std::string _userName{};
    std::string _privateKey{};

    std::map<std::array<uint8_t, CLIENT_ID_SIZE>, Friend> _friends;
    mutable std::mutex _friendsMutex;

};
