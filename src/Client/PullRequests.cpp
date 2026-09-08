#include "PullRequests.h"

#include <thread>
#include <chrono>
#include <iostream>

void pullWaitingMessagesLoop(ClientContext& ctx, ClientConnection& conn, std::atomic<bool>& stopFlag){
    while (!stopFlag.load()) {
        for (int i = 0; i < 60; ++i) {
            if (stopFlag.load()) {
                return;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (stopFlag.load()) {break;}

        if (!ctx.hasClientID()) {continue;}

        try {
            Message msg(ctx.getClientID(), Code::REQUEST_WAITING_MESSAGES);
            auto data = msg.createBuffer();

            auto buffer = conn.sendAndReceive(data);
            std::cout << "Pulling waiting messages automatically:\n";
            responseClientMain(buffer, ctx);
        }
        catch (...) {
            // Do not terminate the client if a periodic pull fails.
        }
    }
}