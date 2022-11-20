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
#include <memory.h>
#include <stdexcept>
#include <string>
#include <thread>

#include "argparse/argparse.hpp"
#include "superstring/superstring.h"

#include "chess/command_dispatcher.h"
#include "chess/position.h"
#include "chess/movegen.h"
#include "chess/stdio_channel.h"

/**
 * @brief PERFormance Test
 */
class Perft final {
public:
    /**
     * @brief Constructor
     * 
     * @param channel Channel to listen for user commands
     */
    explicit Perft(std::shared_ptr<chess::InputStreamChannel> channel)
        : dispatcher_(),
          input_channel_(channel),
          max_depth_(0),
          position_() {
        dispatcher_.RegisterCommand(
            "divide",
            std::bind(&Perft::HandleCommandDivide, this,
                      std::placeholders::_1));
        dispatcher_.RegisterCommand(
            "help",
            std::bind(&Perft::HandleCommandHelp, this,
                      std::placeholders::_1));
        dispatcher_.RegisterCommand(
            "move",
            std::bind(&Perft::HandleCommandMove, this,
                      std::placeholders::_1));
        dispatcher_.RegisterCommand(
            "perft",
            std::bind(&Perft::HandleCommandPerft, this,
                      std::placeholders::_1));
        dispatcher_.RegisterCommand(
            "position",
            std::bind(&Perft::HandleCommandPosition, this,
                      std::placeholders::_1));
        dispatcher_.RegisterCommand(
            "quit",
            std::bind(&Perft::HandleCommandQuit, this,
                      std::placeholders::_1));

        input_channel_->emit_ =
            std::bind(&chess::CommandDispatcher::HandleCommand,
                      &dispatcher_,
                      std::placeholders::_1);

        position_.Reset();
    }

private:
    /**
     * @brief Handle the "divide" command
     *
     * @param args Arguments to this command
     *
     * @return True on success
     */
    bool HandleCommandDivide(const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::size_t parsed_depth, nodes;

            try {
                parsed_depth = std::stoul(args[0]);
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
                return false;
            }

            const auto start = std::chrono::steady_clock::now();

            if (position_.ToMove() == chess::Player::kWhite) {
                nodes = Divide<chess::Player::kWhite>(&position_, parsed_depth);
            } else {
                nodes = Divide<chess::Player::kBlack>(&position_, parsed_depth);
            }

            const auto stop  = std::chrono::steady_clock::now();

            const std::size_t ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stop - start).count();

            std::cout << "Nodes=" << nodes << " Time=" << ms
                      << "ms" << std::endl;

            return true;
        } else {
            std::cout << "usage: divide <depth>" << std::endl;
            return false;
        }
    }

    /**
     * @brief Handle the "help" command
     *
     * @return True on success
     */
    bool HandleCommandHelp(const std::vector<std::string>& ) {
        return true;
    }

    /**
     * @brief Handle the "move" command
     *
     * @param args Arguments to this command
     *
     * @return True on success
     */
    bool HandleCommandMove(const std::vector<std::string>& args) {
        return true;
    }

    /**
     * @brief Handle the "perft" command
     *
     * @param args Arguments to this command
     *
     * @return True on success
     */
    bool HandleCommandPerft(const std::vector<std::string>& args) {
        if (!args.empty()) {
            std::size_t parsed_depth, nodes;

            try {
                parsed_depth = std::stoul(args[0]);
            } catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
                return false;
            }

            max_depth_ = parsed_depth;

            const auto start = std::chrono::steady_clock::now();

            if (position_.ToMove() == chess::Player::kWhite) {
                nodes = Trace<chess::Player::kWhite>(&position_, 0);
            } else {
                nodes = Trace<chess::Player::kBlack>(&position_, 0);
            }

            const auto stop  = std::chrono::steady_clock::now();

            const std::size_t ms =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    stop - start).count();

            std::cout << "Nodes=" << nodes << " Time=" << ms
                      << "ms" << std::endl;

            return true;
        } else {
            std::cout << "usage: perft <depth>" << std::endl;
            return false;
        }
    }

    /**
     * @brief Handle the "position" command
     *
     * @param args Arguments to this command
     *
     * @return True on success
     */
    bool HandleCommandPosition(const std::vector<std::string>& args) {
        const std::string fen =
            jfern::superstring::build(" ", args.begin(), args.end());

        const chess::Position::FenError error = position_.Reset(fen);

        if (error != chess::Position::FenError::kSuccess) {
            std::cout << "Rejected: " << chess::Position::ErrorToString(error)
                      << std::endl;
            return false;
        }

        return true;
    }

    /**
     * @brief Handle the "quit" command
     *
     * @return True on success
     */
    bool HandleCommandQuit(const std::vector<std::string>& ) {
        input_channel_->Close();
        return true;
    }

    /**
     * Run divide() on the specified player
     *
     * @param pos   The root position
     * @param depth Maximum recursive depth, in plies
     *
     * @return The total node count
     */
    template <chess::Player P>
    std::size_t Divide(chess::Position* pos, std::size_t depth) {
        max_depth_ = depth;

        std::size_t total_nodes = 0;

        std::uint32_t moves[chess::kMaxMoves];
        const std::size_t n_moves = !pos->InCheck<P>() ?
            chess::GenerateLegalMoves<P>(*pos, moves) :
            chess::GenerateCheckEvasions<P>(*pos, moves);

        for (std::size_t i = 0; i < n_moves; i++) {
            const std::uint32_t move = moves[i];

            pos->MakeMove<P>(move, 0);

            const std::uint64_t nodes =
                Trace<chess::util::opponent<P>()>(pos, 1);

            pos->UnMakeMove<P>(move, 0);

            // Display the size of this subtree

            const chess::Square orig = chess::util::ExtractFrom(move);
            const chess::Square dest = chess::util::ExtractTo(move);
            const chess::Piece promoted = chess::util::ExtractPromoted(move);

            std::cout << chess::kSquareStr[orig]
                      << chess::kSquareStr[dest];

            if (promoted != chess::Piece::EMPTY) {
                std::cout << "=" << chess::util::PieceToChar(promoted);
            }

            std::cout << " " << nodes << std::endl;

            total_nodes += nodes;
        }

        std::cout << "Moves=" << n_moves << std::endl;

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
    std::size_t Trace(chess::Position* pos, std::uint32_t depth) {
        std::uint32_t moves[chess::kMaxMoves];

        if (depth >= max_depth_) return 1u;

        const std::size_t n_moves = !pos->InCheck<P>() ?
            chess::GenerateLegalMoves<P>(*pos, moves) :
            chess::GenerateCheckEvasions<P>(*pos, moves);

        if (depth+1 >= max_depth_) return n_moves;

        std::uint64_t nodes = 0;

        for (std::size_t i = 0; i < n_moves; i++) {
            const std::uint32_t move = moves[i];
            pos->MakeMove<P>(move, depth);

            nodes += Trace<chess::util::opponent<P>()>(pos, depth+1);

            pos->UnMakeMove<P>(move, depth);
        }

        return nodes;
    }

    /**
     * Handles user commands
     */
    chess::CommandDispatcher dispatcher_;

    /**
     * Channel to listen for commands
     */
    std::shared_ptr<chess::InputStreamChannel>
        input_channel_;

    /**
     * Maximum recursive trace depth
     */
    std::size_t max_depth_;

    /**
     * The position on which to run perft calculations
     */
    chess::Position position_;
};

/**
 * @brief Parse command line and run this program
 *
 * @param parser Command line parser
 *
 * @return True on success
 */
bool go(const argparse::ArgumentParser& parser) {
    auto channel = std::make_shared<chess::StdinChannel>(true);

    Perft perft(channel);
    while (!channel->IsClosed()) channel->Poll();

    return true;
}

int main(int argc, char** argv) {
    argparse::ArgumentParser parser("perft");

    try {
        parser.parse_args(argc, argv);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
        std::cerr << parser;
        return EXIT_FAILURE;
    }

    return go(parser) ? EXIT_SUCCESS : EXIT_FAILURE;
}
