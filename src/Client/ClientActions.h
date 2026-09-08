/**
 * Implements all client-side actions triggered by user input from the menu.
 *
 * This module is responsible for mapping user commands to specific handlers,
 * constructing protocol-compliant requests, and preparing serialized message
 * buffers to be sent to the server.
 *
 * It interacts with the ClientContext to access and update client data,
 * performs input validation, and handles different message types such as
 * registration, key exchange, and encrypted messaging.
 */

#pragma once
#include "Message.h"
#include "MessageCodes.h"
#include "Utilities.h"
#include "ClientContext.h"

#include <cstdint>
#include <array>
#include <string>
#include <vector>

/**
 * Main dispatcher for handling user menu actions.
 * Maps the selected menu code to the appropriate handler function,
 * builds the corresponding request, and returns a serialized message buffer.
 *
 * @param Receives the current client context and a menu code representing the user's chosen action
 * @return Returns a serialized buffer ready to be sent to the server,
 * or an empty buffer if the operation is cancelled or invalid
 * @throws Does not throw exceptions; errors and invalid states are handled
 * by returning an empty buffer or printing a message to the user
 */
std::vector<uint8_t> handleMenuChoice(ClientContext& ctx, uint16_t code);
