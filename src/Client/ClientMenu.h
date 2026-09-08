/**
 * Handles user interaction through the console menu.
 *
 * This module is responsible for displaying the client menu,
 * receiving user input, validating it, and ensuring that only
 * valid menu options are returned.
 *
 * It provides helper functions for parsing and validating numeric input
 * and mapping it to supported client actions.
 */

#pragma once

#include <cstdint>
#include <string>

#include "MessageCodes.h"

constexpr uint16_t INPUT_ERROR = UINT16_MAX;

/**
 * Displays the main client menu and handles user interaction.
 * Repeatedly prompts the user until a valid menu option is entered.
 *
 * @return Returns a valid menu option selected by the user
 */
uint16_t clientMenuMain();

/**
 * Reads and parses user input from the console.
 * Validates that the input is numeric and within the valid range for uint16_t.
 *
 * @return Returns the parsed numeric value,
 * or INPUT_ERROR if the input is invalid
 */
uint16_t getUserInput();

/**
 * Checks whether a string represents a valid numeric value.
 * Allows leading and trailing whitespace but requires all other characters to be digits.
 *
 * @param Receives a string to validate
 * @return Returns true if the string is a valid number, otherwise false
 */
bool isNumber(const std::string& s);

/**
 * Validates whether a given value corresponds to a supported menu option.
 * Compares the input against predefined menu codes.
 *
 * @param Receives a numeric value to validate
 * @return Returns true if the value is a valid menu option, otherwise false
 */
bool isValidMenuOption(uint16_t num);