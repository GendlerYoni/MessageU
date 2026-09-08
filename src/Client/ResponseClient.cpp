#include "ResponseClient.h"

#include "AESWrapper.h"
#include "Base64Wrapper.h"
#include "Constants.h"
#include "FileManager.h"
#include "MessageCodes.h"
#include "Utilities.h"
#include "RSAWrapper.h"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <stdexcept>
namespace {
	/**
	 * Handles a successful registration response from the server.
	 * Extracts the assigned client ID from the response, updates the client context,
	 * saves the client data to disk, and prints the registration details.
	 *
	 * @param Receives a serialized response buffer and the client context
	 */
	void responseRegister(const std::vector<uint8_t>& data, ClientContext& ctx);

	/**
	 * Handles a client list response from the server.
	 * Parses all client entries in the response, prints their names and IDs,
	 * and updates the local client cache.
	 *
	 * @param Receives a serialized response buffer and the client context
	 */
	void responseClientList(const std::vector<uint8_t>& data, ClientContext& ctx);

	/**
	 * Handles a public key response from the server.
	 * Extracts the client ID and public key, updates the local client cache,
	 * and prints the received key in a readable format.
	 *
	 * @param Receives a serialized response buffer and the client context
	 */
	void responsePublicKey(const std::vector<uint8_t>& data, ClientContext& ctx);

	/**
	 * Handles a message sent confirmation response from the server.
	 * Extracts the target client ID and message ID, and displays a confirmation message.
	 *
	 * @param Receives a serialized response buffer and the client context
	 */
	void responseMessageSent(const std::vector<uint8_t>& data, ClientContext& ctx);

	/**
	 * Handles a response containing waiting messages from the server.
	 * Parses each message entry, identifies its type, and processes it accordingly,
	 * including decrypting symmetric keys and text messages when possible.
	 *
	 * @param Receives a serialized response buffer and the client context
	 */
	void responseWaitingMessages(const std::vector<uint8_t>& data, ClientContext& ctx);

	/**
	 * Handles a general error response from the server.
	 * Displays an error message to the user.
	 */
	void responseErrorGeneral();

	/**
	 * Handles a received symmetric key message.
	 * Decrypts the symmetric key using the client's private RSA key,
	 * validates its size, stores it in the client context, and updates the local cache.
	 *
	 * @param Receives the sender client ID, a pointer to the encrypted content,
	 * the content size, the client context, and the RSA private key wrapper
	 */
	void handleSymmetricKeyMessage(const std::array<uint8_t, CLIENT_ID_SIZE>& fromId, const uint8_t* contentPtr,
		uint32_t msgSize, ClientContext& ctx, RSAPrivateWrapper& rsaPriv);

	/**
	 * Handles a received encrypted text message.
	 * Verifies that a symmetric key exists for the sender, decrypts the message using AES,
	 * and prints the resulting plaintext.
	 *
	 * @param Receives the sender client ID, a pointer to the encrypted content,
	 * the content size, and the client context
	 */
	void handleTextMessage(const std::array<uint8_t, CLIENT_ID_SIZE>& fromId, const uint8_t* contentPtr,
		uint32_t msgSize, ClientContext& ctx);

	/**
	 * Extracts a client ID from a serialized buffer at a given offset.
	 * Copies the required number of bytes and returns them as a fixed-size array.
	 *
	 * @param Receives a data buffer and an offset indicating where the client ID starts
	 * @return Returns the extracted client ID
	 * @throws std::runtime_error If there are not enough bytes in the buffer
	 */
	std::array<uint8_t, CLIENT_ID_SIZE> readClientID(const std::vector<uint8_t>& data, size_t offset);
}
void responseClientMain(const std::vector<uint8_t>& data, ClientContext& ctx) {
	if (data.size() < HEADER_SIZE) {
		throw std::runtime_error("responseClientMain: response too small");
	}
	uint16_t code = 0;
	code |= static_cast<uint16_t>(data[RESPONSE_CODE_OFFSET]);
	code |= static_cast<uint16_t>(data[RESPONSE_CODE_OFFSET+1]) << 8;

	switch (code) {
	case Code::RESPONSE_REGISTER: return responseRegister(data, ctx);
	case Code::RESPONSE_CLIENT_LIST: return responseClientList(data, ctx);
	case Code::RESPONSE_PUBLIC_KEY: return responsePublicKey(data, ctx);
	case Code::RESPONSE_MESSAGE_SENT: return responseMessageSent(data, ctx);
	case Code::RESPONSE_WAITING_MESSAGES: return responseWaitingMessages(data, ctx);
	case Code::ERROR_GENERAL: return responseErrorGeneral();
	default:
		std::cout << "Unknown response code: " << code << "\n";
		return;
	}
}

namespace {
	void responseRegister(const std::vector<uint8_t>& data, ClientContext& ctx) {
		std::array<uint8_t, CLIENT_ID_SIZE> clientID = readClientID(data, HEADER_SIZE);
		ctx.setClientID(clientID);
		saveMyInfo(ctx);
		std::cout << "----- Client Context -----\n";

		std::cout << "Name: " << ctx.getName() << "\n";

		std::cout << "Client ID: ";
		for (uint8_t b : ctx.getClientID()) {
			std::cout << std::hex << std::setw(2) << std::setfill('0')
				<< static_cast<int>(b);
		}
		std::cout << std::dec << "\n";
	}

	void responseClientList(const std::vector<uint8_t>& data, ClientContext& ctx) {
		const size_t payloadSize = data.size() - HEADER_SIZE;

		if (payloadSize % (CLIENT_ID_SIZE + NAME_SIZE) != 0) {
			throw std::runtime_error("responseClientList: invalid payload size for 2101");
		}

		for (size_t offset = HEADER_SIZE; offset < data.size(); offset += CLIENT_ID_SIZE + NAME_SIZE) {

			std::array<uint8_t, CLIENT_ID_SIZE> clientID = readClientID(data, offset);
			std::string idHex = bytesToHex(clientID);

			const char* namePtr = reinterpret_cast<const char*>(data.data() + offset + CLIENT_ID_SIZE);
			std::string name(namePtr, strnlen(namePtr, NAME_SIZE));

			std::cout << name << " : " << idHex << "\n";

			ctx.setFriendBase(clientID, name);

		}
	}
	
	void responsePublicKey(const std::vector<uint8_t>& data, ClientContext& ctx) {
		if (data.size() < HEADER_SIZE + CLIENT_ID_SIZE + PUBLIC_KEY_SIZE) {
			throw std::runtime_error("responsePublicKey: response too small");
		}


		std::array<uint8_t, CLIENT_ID_SIZE> clientID = readClientID(data, HEADER_SIZE);
		std::string idHex = bytesToHex(clientID);

		std::array<uint8_t, PUBLIC_KEY_SIZE> publicKey{};
		std::memcpy(publicKey.data(),data.data() + HEADER_SIZE + CLIENT_ID_SIZE,PUBLIC_KEY_SIZE);

		if (!ctx.tryGetFriendCopy(clientID)) {
			ctx.setFriendBase(clientID, "");
		}

		std::string pkBinary(reinterpret_cast<const char*>(publicKey.data()), publicKey.size());

		ctx.setFriendPublicKey(clientID, publicKey);
		std::string pkBase64 = Base64Wrapper::encode(pkBinary);

		std::cout << "Received public key for " << idHex << "\n";
		std::cout << "Public Key (Base64):\n" << pkBase64 << "\n";
	}

	void responseMessageSent(const std::vector<uint8_t>& data, ClientContext& ctx) {

		if (data.size() < HEADER_SIZE + CLIENT_ID_SIZE + MSG_ID_SIZE) {
			throw std::runtime_error("responseMessageSent: response too small");
		}

		std::array<uint8_t, CLIENT_ID_SIZE> clientID = readClientID(data, HEADER_SIZE);
		auto targetOpt = ctx.tryGetFriendCopy(clientID);
		if (!targetOpt) {
			throw std::logic_error("responseMessageSent: friend not found in cache");
		}

		const std::string& displayName = targetOpt->friendName;

		uint32_t msgID = readUint32LE(data, HEADER_SIZE + CLIENT_ID_SIZE);

		std::cout << "Your message has been sent to " << displayName
			<< " and assigned " << msgID << " message ID.\n";
	}

	void responseWaitingMessages(const std::vector<uint8_t>& data, ClientContext& ctx) {

		size_t offset = HEADER_SIZE;

		if (offset == data.size()) {
			std::cout << "No waiting messages.\n";
			return;
		}

		RSAPrivateWrapper rsaPriv(ctx.getPrivateKey());

		while (offset < data.size()) {

			constexpr size_t RECORD_MIN = CLIENT_ID_SIZE + MSG_ID_SIZE + TYPE_SIZE + MSG_SIZE;
			if (data.size() < offset + RECORD_MIN) {
				throw std::runtime_error("responseWaitingMessages: truncated record header");
			}

			std::array<uint8_t, CLIENT_ID_SIZE> fromId = readClientID(data, offset);
			offset += CLIENT_ID_SIZE;

			offset += MSG_ID_SIZE;

			uint8_t msgType = data[offset];
			offset += TYPE_SIZE;

			uint32_t msgSize = readUint32LE(data, offset);
			offset += MSG_SIZE;

			if (data.size() < offset + msgSize) {
				throw std::runtime_error("responseWaitingMessages: truncated message content");
			}

			const uint8_t* contentPtr = (msgSize > 0) ? (data.data() + offset) : nullptr;
			offset += msgSize;

			std::string fromName;
	
			auto frOpt = ctx.tryGetFriendCopy(fromId);

			if (frOpt && !frOpt->friendName.empty())
				fromName = frOpt->friendName;
			else
				fromName = bytesToHex(fromId);

			std::cout << "From: " << fromName << "\n";
			std::cout << "Content:\n";

			// --- TYPE 1: Request symmetric key ---
			if (msgType == MessageType::REQUEST_SYMMETRIC_KEY) {
				std::cout << "Request for symmetric key\n";
			}

			// --- TYPE 2: Send symmetric key (RSA encrypted) ---
			else if (msgType == MessageType::SEND_SYMMETRIC_KEY) {
				handleSymmetricKeyMessage(fromId, contentPtr, msgSize, ctx, rsaPriv);
			}

			// --- TYPE 3: Text message (AES encrypted) ---
			else if (msgType == MessageType::SEND_TEXT) {
				handleTextMessage(fromId, contentPtr, msgSize, ctx);
			}

			else {
				std::cout << "[Unsupported message type: " << static_cast<int>(msgType) << "]\n";
			}

			std::cout << "-----<EOM>-----\n\n";
		}
	}
	void handleSymmetricKeyMessage(const std::array<uint8_t, CLIENT_ID_SIZE>& fromId, const uint8_t* contentPtr,
		uint32_t msgSize, ClientContext& ctx, RSAPrivateWrapper& rsaPriv){
		try {
			std::string enc(reinterpret_cast<const char*>(contentPtr), msgSize);
			std::string symKeyBin = rsaPriv.decrypt(enc);

			if (symKeyBin.size() != SYMMETRIC_KEY_SIZE) {
				std::cout << "can’t decrypt message\n";
				return;
			}

			std::array<uint8_t, SYMMETRIC_KEY_SIZE> symKey{};
			std::memcpy(symKey.data(), symKeyBin.data(), SYMMETRIC_KEY_SIZE);

			if (!ctx.tryGetFriendCopy(fromId)) {
				ctx.setFriendBase(fromId, "");
			}

			ctx.setFriendSymmetricKey(fromId, symKey);
			std::cout << "symmetric key received\n";
		}
		catch (const std::exception&) {
			std::cout << "can’t decrypt message\n";
		}
	}

	void handleTextMessage(const std::array<uint8_t, CLIENT_ID_SIZE>& fromId, const uint8_t* contentPtr,
		uint32_t msgSize, ClientContext& ctx){
		auto frOpt = ctx.tryGetFriendCopy(fromId);

		if (!frOpt || !frOpt->symmetricKey.has_value()) {
			std::cout << "can’t decrypt message\n";
			return;
		}

		try {
			const auto& keyArr = frOpt->symmetricKey.value();

			AESWrapper aes(keyArr.data(), SYMMETRIC_KEY_SIZE);

			std::string enc(reinterpret_cast<const char*>(contentPtr), msgSize);
			std::string txt = aes.decrypt(enc.data(), static_cast<unsigned int>(enc.size()));

			std::cout << txt << "\n";
		}
		catch (const std::exception&) {
			std::cout << "can’t decrypt message\n";
		}
	}


	void responseErrorGeneral() {
		std::cout << "server responded with an error\n";
	}

	std::array<uint8_t, CLIENT_ID_SIZE> readClientID(const std::vector<uint8_t>& data, size_t offset){
		if (data.size() < offset + CLIENT_ID_SIZE) {
			throw std::runtime_error("readClientID: Not enough bytes for ClientID");
		}

		std::array<uint8_t, CLIENT_ID_SIZE> id{};
		std::copy(data.begin() + offset, data.begin() + offset + CLIENT_ID_SIZE, id.begin());

		return id;
	}
}
