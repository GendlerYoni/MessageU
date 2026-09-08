/**
 * Handles periodic pulling of waiting messages from the server.
 *
 * This module runs a background loop that continuously checks for new messages
 * at fixed time intervals and processes them automatically.
 *
 * It uses a stop flag for controlled termination, interacts with the server
 * through the connection module, and updates the client state accordingly.
 */

#pragma once

#include "ClientContext.h"
#include "ClientConnection.h"
#include "Message.h"
#include "MessageCodes.h"
#include "ResponseClient.h"

#include <atomic>

 /**
  * Runs a background loop that periodically requests waiting messages from the server.
  * Waits for a 60 seconds between requests, checks if the user the logged in,
  * sends a request, and processes the received messages.
  *
  * @param Receives the client context, connection object, and a stop flag used to terminate the loop
  */
void pullWaitingMessagesLoop(ClientContext& ctx, ClientConnection& conn, std::atomic<bool>& stopFlag);