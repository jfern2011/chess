/**
 *  \file   movelist.cc
 *  \author Jason Fernandez
 *  \date   01/09/2022
 */

#include "chess/movelist.h"

namespace chess {
/**
 * @brief Constructor
 *
 * @param ptr The first item in the container
 */
MoveList::Iterator::Iterator(std::int32_t* ptr) : ptr_(ptr) {
}

/**
 * @brief Dereference operator
 * 
 * @return The current item being pointed to
 */
std::int32_t MoveList::Iterator::operator*() const noexcept {
    return *ptr_;
}

/**
 * @brief Prefix increment
 *
 * @return *this pointing to the next element
 */
auto MoveList::Iterator::operator++() noexcept -> Iterator& {
    ptr_++; return *this;
}

/**
 * @brief Postfix increment
 * 
 * @return A copy of *this prior to the increment
 */
auto MoveList::Iterator::operator++(int) noexcept -> Iterator {
    Iterator prev = *this; ptr_++;
    return prev;
}

/**
 * @brief Compare two iterators for equality
 *
 * @param a The 1st operand
 * @param b The 2nd operand
 *
 * @return True if they are equal, false otherwise
 */
bool operator==(const MoveList::Iterator& a, const MoveList::Iterator& b) {
    return a.ptr_ == b.ptr_;
};

/**
 * @brief Compare two iterators for inequality
 *
 * @param a The 1st operand
 * @param b The 2nd operand
 *
 * @return True if they are NOT equal, false otherwise
 */
bool operator!=(const MoveList::Iterator& a, const MoveList::Iterator& b) {
    return a.ptr_ != b.ptr_;
};

/**
 * @brief Constructor
 */
MoveList::MoveList() : moves_(), size_(0) {
}

/**
 * @brief Add a move to the end of the list
 * 
 * @param move The move to add
 */
void MoveList::Append(std::int32_t move) noexcept {
    moves_[size_++] = move;
}

/**
 * @brief Clear the list of moves
 */
void MoveList::Clear() noexcept {
    size_ = 0;
}

/**
 * @brief Get an iterator to the start of the list
 *
 * @return An iterator to the first move
 */
auto MoveList::Begin() noexcept -> Iterator {
    return Iterator(&moves_[0]);
}

/**
 * @brief Get an iterator to the end of the list
 *
 * @return An iterator pointing immediately after the last move
 */
auto MoveList::End() noexcept -> Iterator   {
    return Iterator(&moves_[size_]);
}

}  // namespace chess
