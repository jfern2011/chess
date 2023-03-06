/**
 *  \file   engine_main.cc
 *  \author Jason Fernandez
 *  \date   01/14/2023
 */

#include <array>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>

#include "argparse/argparse.hpp"
#include "chess/engine.h"
#include "chess/file_stream.h"
#include "chess/logger.h"
#include "chess/stdio_channel.h"
#include "chess/uci.h"

/**
 * @brief Parse the command line and run this program
 *
 * @return True on success
 */
bool go(const argparse::ArgumentParser& ) {
    // For now, read only from stdin and direct all output to a text file

    auto input_channel = std::make_shared<chess::StdinChannel>(
                            true /* synced */);

    auto output_channel = std::make_shared<chess::StdoutChannel>();

    std::array<char, 256> prefix{ 0 };

    std::time_t time = std::time({});
    std::strftime(prefix.data(), prefix.size(), "%F-%T-GMT",
                  std::gmtime(&time));

    const std::string fullname = std::string(prefix.data()) + "_log.txt";

    auto logging_channel = std::make_shared<chess::FileStream>(fullname);

    if (!logging_channel->Good()) {
        return false;
    } else {
        (*logging_channel) << "Version 1.0\n";
    }

    auto engine = std::make_shared<chess::Engine>(
        output_channel,
        std::make_shared<chess::Logger>("engine", logging_channel));

    chess::UciProtocol protocol(
        input_channel,
        std::make_shared<chess::Logger>("uci", logging_channel), engine);

    while (!input_channel->IsClosed()) input_channel->Poll();

    return true;
}

/**
 * @brief Entry point
 *
 * @param argc Number of command line arguments
 * @param argv The command line arguments
 *
 * @return Either EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char** argv) {
    argparse::ArgumentParser parser(argv[0]);

    auto logger = std::make_shared<chess::Logger>(
                    "exec", std::make_shared<chess::StdoutChannel>());

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& error) {
        std::ostringstream oss;
        oss << error.what() << std::endl << parser;

        logger->Write(oss.str().c_str());
        return EXIT_FAILURE;
    }

    return go(parser) ? EXIT_SUCCESS : EXIT_FAILURE;
}
