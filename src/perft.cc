#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "argparse/argparse.hpp"

int main(int argc, char** argv) {
    argparse::ArgumentParser parser("perft");

    parser.add_argument("--depth")
        .help("The maximum recursive trace depth, in plies")
        .required();
    parser.add_argument("--divide")
        .help("If true, generate divide() results")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("--fen")
        .help("The root position in Forsyth-Edwards notation")
        .default_value("");
    
    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        std::cerr << parser;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
