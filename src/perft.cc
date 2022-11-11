/**
 *  \file   perft.cc
 *  \author Jason Fernandez
 *  \date   11/10/2022
 */

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include "argparse/argparse.hpp"

#include "chess/position.h"
#include "chess/movegen.h"

/**
 * @brief PERFormance Test
 */
class Perft final {
public:
    /**
     * Run the performance test, computing the number of nodes
     * in the subtree below each move
     *
     * @param pos   The root position
     * @param depth Maximum recursive depth, in plies
     *
     * @return The total node count
     */
    std::size_t Divide(chess::Position* pos, std::size_t depth) {
        if (pos->ToMove() == chess::Player::kWhite) {
            return Divide_<chess::Player::kWhite>(pos, depth);
        } else {
            return Divide_<chess::Player::kBlack>(pos, depth);
        }
    }

    /**
     * Run the performance test
     *
     * @param pos   The root position
     * @param depth Maximum recursive depth, in plies
     *
     * @return The total node count
     */
    std::size_t Run(chess::Position* pos, std::size_t depth) {
        max_depth_ = depth;
        node_count_ = 0;

        if (pos->ToMove() == chess::Player::kWhite) {
            Trace<chess::Player::kWhite>(pos, 0);
        } else {
            Trace<chess::Player::kBlack>(pos, 0);
        }

        return node_count_;
    }

private:
    /**
     * Run divide() on the specified player
     *
     * @param pos   The root position
     * @param depth Maximum recursive depth, in plies
     *
     * @return The total node count
     */
    template <chess::Player P>
    std::size_t Divide_(chess::Position* pos, std::size_t depth) {
        max_depth_ = depth == 0 ? depth : (depth-1);

        std::size_t total_nodes = 0;

        std::uint32_t moves[chess::kMaxMoves];
        const std::size_t n_moves = !pos->InCheck<P>() ?
            chess::GenerateLegalMoves<P>(*pos, moves) :
            chess::GenerateCheckEvasions<P>(*pos, moves);

        for (std::size_t i = 0; i < n_moves; i++) {
            node_count_ = 0;

            const std::uint32_t move = moves[i];

            pos->MakeMove<P>(move, 0);

            Trace<chess::util::opponent<P>()>(pos, 1);

            pos->UnMakeMove<P>(move, 0);

            chess::Square orig = chess::util::ExtractFrom(move);
            chess::Square dest = chess::util::ExtractTo(move);

            std::cout << chess::kSquareStr[orig]
                      << chess::kSquareStr[dest] << ": " << node_count_
                      << std::endl;

            total_nodes += node_count_;
        }

        return total_nodes;
    }

    /**
     * @brief Internal recursive routine
     *
     * @tparam P The player whose turn it is at this depth
     *
     * @param pos   The current position
     * @param depth The current depth
     */
    template <chess::Player P>
    void Trace(chess::Position* pos, std::uint32_t depth) {
        if (depth >= max_depth_) {
            node_count_++;
        } else {
            std::uint32_t moves[chess::kMaxMoves];
            const std::size_t n_moves = !pos->InCheck<P>() ?
                chess::GenerateLegalMoves<P>(*pos, moves) :
                chess::GenerateCheckEvasions<P>(*pos, moves);

            if (n_moves == 0) {
                node_count_++;
            } else {
                for (std::size_t i = 0; i < n_moves; i++) {
                    const std::uint32_t move = moves[i];
                    pos->MakeMove<P>(move, depth);

                    Trace<chess::util::opponent<P>()>(pos, depth+1);

                    pos->UnMakeMove<P>(move, depth);
                }
            }
        }
    }

    /**
     * Maximum recursive trace depth
     */
    std::size_t max_depth_;

    /**
     * The total number of nodes visited
     */
    std::size_t node_count_;
};

/**
 * @brief Parse command line and run this program
 *
 * @param parser Command line parser
 *
 * @return True on success
 */
bool go(const argparse::ArgumentParser& parser) {
    const auto max_depth = parser.get<std::size_t>("--depth");
    const auto do_divide = parser.get<bool>("--divide");
    const auto fen       = parser.get<std::string>("--fen");

    chess::Position position;
    chess::Position::FenError error = position.Reset(fen);
    if (error != chess::Position::FenError::kSuccess) {
        std::cout << chess::Position::ErrorToString(error) << std::endl;
        return false;
    }

    Perft perft;

    const auto start = std::chrono::steady_clock::now();
    const std::size_t node_count =
        do_divide ? perft.Divide(&position, max_depth) :
                    perft.Run(&position, max_depth);
    const auto stop  = std::chrono::steady_clock::now();

    std::size_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        start - stop).count();

    std::cout << "Nodes=" << node_count << " Time=" << ms << "ms"
              << std::endl;

    return true;
}

int main(int argc, char** argv) {
    argparse::ArgumentParser parser("perft");

    parser.add_argument("--depth")
        .help("The maximum recursive trace depth, in plies")
        .scan<'u', std::size_t>()
        .required();
    parser.add_argument("--divide")
        .help("If true, generate divide() results")
        .default_value(false)
        .implicit_value(true);
    parser.add_argument("--fen")
        .help("The root position in Forsyth-Edwards notation")
        .default_value(std::string());

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        std::cerr << parser;
        return EXIT_FAILURE;
    }

    return go(parser) ? EXIT_SUCCESS : EXIT_FAILURE;
}
