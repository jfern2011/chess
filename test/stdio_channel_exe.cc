/**
 *  \file   stdio_channel_ut.cc
 *  \author Jason Fernandez
 *  \date   12/26/2021
 */

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include "chess/stdio_channel.h"

namespace {
/**
 * @brief Get the current time
 */
std::string now() {
    std::time_t t = std::time(nullptr);
    char mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "%A %c", std::localtime(&t));

    return std::string(mbstr);
}

}  // namespace

int main(int argc, char** argv) {
    chess::StdinChannel channel;
    std::string input;
    std::mutex input_mutex;

    channel.emit_ =
        [&] (const chess::ConstDataBuffer& buf) {
        input_mutex.lock();
        input = std::string(buf.data(), buf.size());
        input_mutex.unlock();
    };

    constexpr int sleep_period = 1;

    while (true) {
        channel.Poll();

        std::cout << now() << ": ";

        input_mutex.lock();

        if (input.find("quit") != std::string::npos) {
            std::cout << std::endl;
            break;
        }

        if (input.empty()) {
            std::cout << "<no data>";
        } else {
            std::cout << "New message: '" << input << "'";
            input.clear();
        }

        std::cout << std::endl;

        input_mutex.unlock();

        std::this_thread::sleep_for(
            std::chrono::seconds(sleep_period));
    }

    return 0;
}
