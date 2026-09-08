#include "ClientMenu.h"

#include <string>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <cctype>

uint16_t clientMenuMain() {
	std::string menu = "MessageU client at your service. \n\n"
		"110) Register\n"
		"120) Request for clients list\n"
		"130) Request for public key\n"
		"140) Request for waiting messages\n"
		"150) Send a text message\n"
		"151) Send a request for symmetric key\n"
		"152) Send your symmetric key\n"
		"0) Exit client\n"
		"?\n";
    while (true) {
        std::cout << menu;
        uint16_t input = getUserInput();
        if (input == INPUT_ERROR || !isValidMenuOption(input)) {
            std::cout << "Invalid input, please try again.\n";
            continue;
        }
        return input;
    }
}

uint16_t getUserInput() {
    std::string input;
    std::getline(std::cin, input);
    if (!isNumber(input)) { return INPUT_ERROR; }

    unsigned long value = std::strtoul(input.c_str(), nullptr, 10);

    if (value > UINT16_MAX) { return INPUT_ERROR; }

    return static_cast<uint16_t>(value);
}

bool isNumber(const std::string& s) {
    size_t i = 0;

    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        i++;
    }

    if (i == s.size()) { return false; }

    while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i]))) {
        i++;
    }

    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
        i++;
    }

    return i == s.size();
}

bool isValidMenuOption(uint16_t num) {
    switch (num) {
    case Code::REGISTER:
    case Code::CLIENTS_LIST:
    case Code::PUBLIC_KEY:
    case Code::WAITING_MESSAGES:
    case Code::SEND_TEXT:
    case Code::REQUEST_SYMMETRIC_KEY:
    case Code::SEND_SYMMETRIC_KEY:
    case Code::EXIT:
        return true;
    default:
        return false;
    }
}