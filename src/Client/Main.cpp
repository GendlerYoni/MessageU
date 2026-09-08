/**
 * This program implements the client side of the MessageU messaging system.
 * It allows users to register, request client data, exchange public and symmetric keys,
 * send encrypted messages, and retrieve waiting messages from the server.
 *
 * The client communicates with the server using a custom binary protocol,
 * stores user information locally, and supports end-to-end encryption using RSA and AES.
 *
 * The main function initializes the client state, loads saved configuration,
 * establishes the server connection, starts the background pull thread,
 * and runs the main interaction loop until the user exits the program.
 */

#include <iostream>
#include <exception>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>

#include "MessageCodes.h"
#include "ClientMenu.h"
#include "ClientActions.h"
#include "ClientConnection.h"
#include "PullRequests.h"
#include "FileManager.h"


 /**
  * Runs the main client workflow.
  * Initializes the client context, loads saved data, connects to the server,
  * starts the background pull thread, and handles user actions until exit.
  *
  * @return Returns 0 on successful termination, or 1 if a fatal error occurs
  */
int main()
{
    std::atomic<bool> stopFlag = false;

    try {
        ClientContext clientCT;
        loadMyInfo(clientCT);
        ClientConnection conn;
        std::thread pullThread(pullWaitingMessagesLoop, std::ref(clientCT),
            std::ref(conn), std::ref(stopFlag));

        try {
            while (true) {
                uint16_t input = clientMenuMain();

                if (input == Code::EXIT) {
                    std::cout << "Exiting client...\n";
                    stopFlag = true;
                    pullThread.join();
                    return 0;
                }

                std::vector<uint8_t> data = handleMenuChoice(clientCT, input);
                if (data.empty()) continue;

                auto buffer = conn.sendAndReceive(data);
                responseClientMain(buffer, clientCT);
            }
        }
        catch (...) {
            stopFlag = true;
            if (pullThread.joinable()) {
                pullThread.join();
            }
            throw;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
