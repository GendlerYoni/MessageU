#include "ClientActions.h"

#include "RSAWrapper.h"
#include "AESWrapper.h"


#include <algorithm>
#include <cstring>
#include <iostream>

namespace {
	/**
	 * Handles the user registration process.
	 * Prompts the user for a username, generates RSA key pairs,
	 * builds a registration request payload, and prepares a message
	 * to be sent to the server.
	 *
	 * @param Receives the client context used to store the username and generated keys
	 * @return Returns a serialized buffer containing the registration request,
	 * or an empty buffer if the user is already registered or the operation is cancelled
	 * @throws Does not throw exceptions; errors and invalid states are handled
	 * by returning an empty buffer or printing a message to the user
	 */
	std::vector<uint8_t> handleRegister(ClientContext& ctx);

	/**
	 * Handles a request to retrieve the list of registered clients.
	 * Builds a protocol-compliant request message using the client's ID.
	 *
	 * @param Receives the client context containing the client ID
	 * @return Returns a serialized buffer representing the clients list request
	 */
	std::vector<uint8_t> handleClientList(const ClientContext& ctx);

	/**
	 * Handles a request to retrieve the public key of another client.
	 * Prompts the user for a target username, validates it against the local cache,
	 * and builds a protocol-compliant request message containing the target client ID.
	 *
	 * @param Receives the client context used to access cached client data
	 * @return Returns a serialized buffer representing the public key request,
	 * or an empty buffer if the user cancels or the target user is not found
	 */

	std::vector<uint8_t> handlePublicKey(ClientContext& ctx);
	/**
	 * Handles a request to retrieve all waiting messages for the current client.
	 * Builds a protocol-compliant request message using the client's ID.
	 *
	 * @param Receives the client context containing the client ID
	 * @return Returns a serialized buffer representing the waiting messages request
	 */
	std::vector<uint8_t> handleWaitingMessages(const ClientContext& ctx);

	/**
	 * Handles all message-sending operations.
	 * Prompts the user for a target username, validates it, builds the appropriate
	 * message payload based on the selected action, and prepares a request to be sent to the server.
	 *
	 * @param Receives the client context and a code representing the type of message operation
	 * @return Returns a serialized buffer representing the message request,
	 * or an empty buffer if the user cancels or validation fails
	 */
	std::vector<uint8_t> handleAllMessages(ClientContext& ctx, uint16_t code);

	/**
	 * Handles sending an encrypted text message to another client.
	 * Collects multi-line input from the user, validates the existence of a symmetric key,
	 * encrypts the message using AES, and prepares the message content.
	 *
	 * @param Receives the client context and the target client ID
	 * @return Returns a byte vector containing the encrypted message content,
	 * or an empty buffer if the input is empty or required data is missing
	 */
	std::vector<uint8_t> handleSendText(ClientContext& ctx, std::array<uint8_t, CLIENT_ID_SIZE> otherClientID);

	/**
	 * Handles sending a symmetric key to another client.
	 * Generates a new AES key, stores it in the client context,
	 * encrypts it using the target client's public key, and prepares the message content.
	 *
	 * @param Receives the client context and the target client ID
	 * @return Returns a byte vector containing the encrypted symmetric key,
	 * or an empty buffer if the target client does not have a public key
	 */
	std::vector<uint8_t> handleSendSymmetricKey(ClientContext& ctx, std::array<uint8_t, CLIENT_ID_SIZE> otherClientID);

	/**
	 * Prompts the user to enter a valid username and validates the input.
	 * Ensures the username is ASCII-only and within the allowed length.
	 *
	 * @param Receives a prompt string displayed to the user
	 * @return Returns a valid username string,
	 * or "0" if the user chooses to cancel the operation
	 */
	std::string getValidUsername(const std::string& prompt);
}

std::vector<uint8_t> handleMenuChoice(ClientContext& ctx, uint16_t code) {
	switch (code)
	{
	case Code::REGISTER:				return handleRegister(ctx);
	case Code::CLIENTS_LIST:			return handleClientList(ctx);
	case Code::PUBLIC_KEY:				return handlePublicKey(ctx); 
	case Code::WAITING_MESSAGES:		return handleWaitingMessages(ctx);
	case Code::SEND_TEXT:				return handleAllMessages(ctx, Code::SEND_TEXT);
	case Code::REQUEST_SYMMETRIC_KEY:	return handleAllMessages(ctx, Code::REQUEST_SYMMETRIC_KEY);
	case Code::SEND_SYMMETRIC_KEY:		return handleAllMessages(ctx, Code::SEND_SYMMETRIC_KEY);
	default:
		return {};
	}
	return {};
}

namespace {
	std::vector<uint8_t> handleRegister(ClientContext& ctx) {
		if (ctx.hasClientID()) { 
			std::cout << "You are already logged in.\n";
			return {};
		}

		std::string name = getValidUsername("Please enter your username: ");

		ctx.setName(name);

		std::array<uint8_t, NAME_SIZE> nameField{};
		std::memcpy(nameField.data(), name.data(), std::min(name.size(), NAME_SIZE - 1));
		nameField[name.size()] = 0;

		std::vector<uint8_t> payload;
		payload.insert(payload.end(), nameField.begin(), nameField.end());

		RSAPrivateWrapper rsaKeys;

		std::string privateKeyRaw = rsaKeys.getPrivateKey();   
		ctx.setPrivateKey(privateKeyRaw);                      

		std::array<uint8_t, RSAPublicWrapper::KEYSIZE> publicKey{};
		rsaKeys.getPublicKey(reinterpret_cast<char*>(publicKey.data()), RSAPublicWrapper::KEYSIZE);

		payload.insert(payload.end(), publicKey.begin(), publicKey.end());

		Message msg(ctx.getClientID(), Code::REQUEST_REGISTER, payload);
		return msg.createBuffer();
	}

	std::vector<uint8_t> handleClientList(const ClientContext& ctx) {
		Message msg(ctx.getClientID(), Code::REQUEST_CLIENTS_LIST);
		return msg.createBuffer();
	}

	std::vector<uint8_t> handlePublicKey(ClientContext& ctx) { 
		std::string targetName = getValidUsername("Please enter target username (0 to cancel): ");
		if (targetName == "0") return {};

		auto idOpt = ctx.tryGetFriendIdByName(targetName);

		if (!idOpt) {
			std::cout << "User not found in cache. Please request clients list (120) first.\n";
			return {};
		}

		std::vector<uint8_t> payload;
		payload.insert(payload.end(), idOpt->begin(), idOpt->end());

		Message msg(ctx.getClientID(), Code::REQUEST_PUBLIC_KEY, payload);
		return msg.createBuffer();
	}

	std::vector<uint8_t> handleWaitingMessages(const ClientContext& ctx) {
		Message msg(ctx.getClientID(), Code::REQUEST_WAITING_MESSAGES);
		return msg.createBuffer();
	}

	std::vector<uint8_t> handleAllMessages(ClientContext& ctx, uint16_t code) {
		std::string targetName = getValidUsername("Please enter target username (0 to cancel): ");
		if (targetName == "0") return {};

		auto idOpt = ctx.tryGetFriendIdByName(targetName);
		if (!idOpt) {
			std::cout << "User not found. Please request clients list (120) first.\n";
			return {};
		}

		std::array<uint8_t, CLIENT_ID_SIZE> otherClientID = *idOpt;

		std::vector<uint8_t> payload;
		payload.insert(payload.end(), otherClientID.begin(), otherClientID.end());

		std::vector<uint8_t> msgContent; //The specific function

		if (code == Code::SEND_TEXT) {
			msgContent = handleSendText(ctx, otherClientID);
			if (msgContent.empty()) return {};
			payload.push_back(MessageType::SEND_TEXT); //Message type 3
		}

		else if (code == Code::REQUEST_SYMMETRIC_KEY) {
			payload.push_back(MessageType::REQUEST_SYMMETRIC_KEY); //Message type 1
		}
		else if (code == Code::SEND_SYMMETRIC_KEY) {
			msgContent = handleSendSymmetricKey(ctx, otherClientID);
			if (msgContent.empty()) return {};
			payload.push_back(MessageType::SEND_SYMMETRIC_KEY); //Message type 2
		}

		appendUint32LE(payload, static_cast<uint32_t>(msgContent.size()));
		payload.insert(payload.end(), msgContent.begin(), msgContent.end());

		Message msg(ctx.getClientID(), Code::REQUEST_ALL_MESSAGES, payload);
		return msg.createBuffer();
	}

	//Message type 3
	std::vector<uint8_t> handleSendText(ClientContext& ctx, std::array<uint8_t, CLIENT_ID_SIZE> otherClientID) {
		std::cout << "Enter your message (empty line to finish):\n";

		std::string line;
		std::string msg;

		while (std::getline(std::cin, line)){
			if (line.empty())
				break;

			msg += line;
			msg += '\n';
		}

		if (msg.empty()) {
			std::cout << "Empty message. Cancelled.\n";
			return {};
		}

		auto frOpt = ctx.tryGetFriendCopy(otherClientID);

		if (!frOpt) {
			std::cout << "User not found. Please request clients list (120) first.\n";
			return {};
		}

		if (!frOpt->symmetricKey.has_value()) {
			std::cout << "No symmetric key for this user. Please use 152 first.\n";
			return {};
		}

		const auto& key16 = frOpt->symmetricKey.value();
		AESWrapper aes(key16.data(), AESWrapper::DEFAULT_KEYLENGTH);

		std::string cipherBin = aes.encrypt(msg.data(), static_cast<unsigned int>(msg.size()));

		return std::vector<uint8_t>(cipherBin.begin(), cipherBin.end());
	}

	std::vector<uint8_t> handleSendSymmetricKey(ClientContext& ctx, std::array<uint8_t, CLIENT_ID_SIZE> otherClientID){
		auto frOpt = ctx.tryGetFriendCopy(otherClientID);

		if (!frOpt || !frOpt->publicKey.has_value()) {
			std::cout << "No public key for this user. Use 130 first.\n";
			return {};
		}

		AESWrapper aes;
		std::array<uint8_t, SYMMETRIC_KEY_SIZE> symKey{}; 
		std::memcpy(symKey.data(), aes.getKey(), SYMMETRIC_KEY_SIZE);

		ctx.setFriendSymmetricKey(otherClientID, symKey);

		RSAPublicWrapper pub(reinterpret_cast<const char*>(frOpt->publicKey->data()),
			RSAPublicWrapper::KEYSIZE);

		std::string cipher = pub.encrypt(reinterpret_cast<const char*>(symKey.data()), SYMMETRIC_KEY_SIZE);
		return std::vector<uint8_t>(cipher.begin(), cipher.end());
	}

	std::string getValidUsername(const std::string& prompt) {
		std::string name;

		while (true) {
			std::cout << prompt;
			std::getline(std::cin, name);

			if (name == "0") {
				std::cout << "Name cannot be 0.\n";
				return "0";
			}

			if (!isValidUsername(name)) {
				std::cout << "Name must be ASCII only and up to 254 characters.\n";
				continue;
			}

			return name;
		}
	}
}







