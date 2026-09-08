/**
 * Handles parsing and processing of server responses.
 *
 * This module is responsible for interpreting protocol-compliant responses,
 * dispatching them to the appropriate handlers, and updating the client state.
 *
 * It processes different response types such as registration, client lists,
 * public keys, message confirmations, and incoming messages, including
 * decryption and validation when required.
 */

#pragma once

#include <vector>
#include "ClientContext.h"

 /**
  * Main dispatcher for handling server responses.
  * Parses the response header, extracts the response code,
  * and routes the data to the appropriate handler function.
  *
  * @param Receives a serialized response buffer and the client context
  * @throws std::runtime_error If the response is too small or malformed
  */
void responseClientMain(const std::vector<uint8_t>& data, ClientContext& ctx);