/**
 * Handles loading, validation, and saving of client configuration files.
 *
 * This module is responsible for managing persistent client data such as
 * user identity, cryptographic keys, and server connection details.
 *
 * It reads and validates data from configuration files, ensures correct format,
 * and updates the client context accordingly. It also handles serialization
 * of client data for saving to disk.
 */

#pragma once

#include "ClientContext.h"

#include <cstdint>
#include <fstream>
#include <string>

 /**
  * Loads client information from the configuration file.
  * Attempts to read and validate stored client data and update the client context.
  *
  * @param Receives the client context to populate with loaded data
  */
void loadMyInfo(ClientContext& ctx);

/**
 * Saves client information to a configuration file.
 * Serializes the username, client ID, and private key and writes them to disk.
 *
 * @param Receives the client context containing the data to save
 */
void saveMyInfo(const ClientContext& ctx);

/**
 * Loads server connection details from the configuration file.
 * Attempts to read and validate the server address and port.
 *
 * @param Receives references to store the server address and port
 */
void loadServerInfo(std::string& address, uint16_t& port);

