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
#include <vector>

#include "argparse/argparse.hpp"
#include "superstring/superstring.h"

#include "chess/command_dispatcher.h"
#include "chess/data_buffer.h"
#include "chess/interactive.h"
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

        dispatcher_.error_callback_ =
            std::bind(&Perft::HandleCommandUnknown, this,
                      std::placeholders::_1);

        input_channel_->emit_ =
            std::bind(&chess::CommandDispatcher::HandleCommand,
                      &dispatcher_,
                      std::placeholders::_1);

        position_.Reset();

        std::cout << "Type \"help\" for options."
                  << std::endl;
    }

private:
    /**
     * @brief Check the depth argument to the perft and divide commands
     *
     * @param depth The selected depth
     *
     * @return True if the depth is allowed, false otherwise
     */
    bool CheckDepth(std::size_t depth) {
        constexpr std::size_t kLimit = chess::kMaxMoves;

        if (depth > kLimit) {
            std::cout << "Depth must be in [0, " << kLimit << "]"
                      << std::endl;
            return false;
        } else {
            return true;
        }
    }

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

            if (!CheckDepth(parsed_depth))
                return false;

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
        const std::string indentx1(4, ' ');
        const std::string indentx2 = indentx1 + indentx1;

        std::cout << "\ncommands:\n";
        std::cout << indentx1 << "divide <depth>\n";
        std::cout << indentx2
                  << "Break down the size of every subtree from the current"
                     " position to the specified depth.\n";
        std::cout << indentx1 << "help\n";
        std::cout << indentx2 << "Display this help menu.\n";
        std::cout << indentx1 << "move <move>\n";
        std::cout << indentx2 << "Make a move from the current position.\n";
        std::cout << indentx1 << "perft <depth>\n";
        std::cout << indentx2
                  << "Compute the number of terminal nodes from the current"
                     " position to the specified depth.\n";
        std::cout << indentx1 << "position <fen>\n";
        std::cout << indentx2
                  << "Set the current position to a FEN-encoded one.\n";
        std::cout << indentx1 << "quit\n";
        std::cout << indentx2 << "Exit this program.\n"
                  << std::endl;

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
        if (args.empty()) {
            std::cout << "usage: move <move>" << std::endl;
            return false;
        } else {
            const std::uint32_t move = ResolveMove(position_, args[0]);
            if (move == chess::kNullMove) {
                std::cout << "Invalid move: \"" << args[0] << "\""
                          << std::endl;
                return false;
            } else {
                position_.ToMove() == chess::Player::kWhite ?
                    position_.MakeMove<chess::Player::kWhite>(move, 0) :
                    position_.MakeMove<chess::Player::kBlack>(move, 0);
            }
        }

        // Specified move is valid
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

            if (!CheckDepth(parsed_depth))
                return false;

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
     * Called back when an unknown command is issued
     *
     * @param buf The command data
     */
    void HandleCommandUnknown(const chess::ConstDataBuffer& buf) {
        const jfern::superstring sstring(std::string(buf.data(), buf.size()));

        const std::vector<std::string> tokens = sstring.split();
        if (!tokens.empty()) {
            std::cout << "Unknown command \'" << tokens[0] << "\'"
                      << std::endl;
        }
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
 * @return True on success
 */
bool go(const argparse::ArgumentParser& ) {
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
