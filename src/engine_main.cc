/**
 *  \file   engine_main.cc
 *  \author Jason Fernandez
 *  \date   01/14/2023
 */

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <sstream>

#include "argparse/argparse.hpp"
#include "chess/engine.h"
#include "chess/logger.h"
#include "chess/stdio_channel.h"
#include "chess/uci.h"

/**
 * @brief Parse the command line and run this program
 *
 * @return True on success
 */
bool go(const argparse::ArgumentParser& ) {
    // For now, read only from stdin and direct all output to stdout/stderr

    auto input_channel = std::make_shared<chess::StdinChannel>(
                            true /* synced */);

    auto output_channel = std::make_shared<chess::StdoutChannel>();
    auto logging_channel = std::make_shared<chess::StderrChannel>();

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
    argparse::ArgumentParser parser("chess");

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
