/**
 *  \file   mtcs.cc
 *  \author Jason Fernandez
 *  \date   04/15/2023
 */

#include "chess/mtcs.h"

#include <array>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <random>

#include "chess/evaluate.h"
#include "chess/movegen.h"

namespace chess {
/**
 * @brief Random number generator helper
 *
 * Generates numbers in the interval [0, N], where N is user specified
 */
class RandomInt final {
public:
    /**
     * Constructor
     *
     * @param max Maximum value that should be generated
     */
    explicit RandomInt(std::size_t max)
        : dist_(), gen_(), max_(max) {
        dist_ = std::make_shared<std::uniform_int_distribution<>>(
                    0, max_);

        std::random_device dev;
        gen_ = std::make_shared<std::mt19937>(dev());
    }

    /**
     * @brief Get the next random value
     *
     * @return An integer in the range [0, N] where N is user specified
     */
    std::size_t next() {
        return dist_->operator()(*gen_);
    }

private:
    /**
     * The actual number generator
     */
    std::shared_ptr<std::uniform_int_distribution<>>
        dist_;
    
    /**
     * The random engine
     */
    std::shared_ptr<std::mt19937> gen_;

    /**
     * Maximum value to be generated, inclusive
     */
    std::size_t max_;
};

/**
 * @brief Generate a random integer
 *
 * @param max_value Generate a value between [0, max_value)
 *
 * @return The random value
 */
std::size_t random(std::size_t max_value) {
    static RandomInt generator(kMaxMoves);

    return generator.next() % max_value;
}

/**
 * @brief Default constructor
 */
Mtcs::Node::Node()
    : childs_(nullptr),
      hash_(0x0),
      next_(nullptr),
      num_childs_(0u),
      sum_(0),
      visits_(0u) {
}

/**
 * @brief Get the average value of this node
 *
 * @return The average value
 */
double Mtcs::Node::Average() const {
    return visits_ == 0u ? kInfinityF64 : double(sum_) / visits_;
}

/**
 * @brief Get the number of times this node has been visited
 *
 * @return The number of visits
 */
std::size_t Mtcs::Node::visits() const {
    return visits_;
}

/**
 * @brief Get the last node in the list of childs
 *
 * @return Node* The last node in the list or nullptr if the list is empty
 */
auto Mtcs::Node::End() -> Node* {
    if (!childs_) {
        return nullptr;
    }

    Node* node = childs_;

    while (node->next_ != nullptr) {
        node = node->next_;
    }

    return node;
}

/**
 * @brief Constructor
 *
 * @param pool   The memory pool from which to allocate nodes
 * @param logger Logs internal info/diagnostics
 */
Mtcs::Mtcs(std::shared_ptr<MemoryPool<Node>> pool,
           std::shared_ptr<Logger> logger)
    : childs_(),
      iterations_(0),
      logger_(logger),
      node_pool_(pool) {
}

/**
 * @see Search::Run()
 *
 * @todo Clear memory pool between calls to this method?
 */
std::uint32_t Mtcs::Run(const Position& position) {
    if (GameResult(position) != Result::kGameNotOver) {
        return kNullMove;
    }

    const std::size_t n_iterations = 10000;

    Position pos(position);

    std::pair<double, std::uint32_t> result;

    for (std::size_t iter = 0; iter <= n_iterations; iter++) {
        result = pos.ToMove() == Player::kWhite ? 
                                    SelectRoot<Player::kWhite>(&pos) :
                                    SelectRoot<Player::kBlack>(&pos);

        if (node_pool_->Full()) {
            logger_->Write("Ran out of memory after %zu iteration(s)",
                           iter+1);
        }
    }

    return result.second;
}

}  // namespace chess
