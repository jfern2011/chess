/**
 *  \file   movelist.h
 *  \author Jason Fernandez
 *  \date   01/09/2022
 */

#ifndef CHESS_MOVELIST_H_
#define CHESS_MOVELIST_H_

#include <array>
#include <cstddef>
#include <cstdint>

namespace chess {
/**
 * @brief A list of moves, each encoded in 21 bits
 */
class MoveList final {
public:
    /**
     * @brief Iterates over a MoveList
     */
    class Iterator final {
    public:
        explicit Iterator(std::int32_t* ptr);

        Iterator(const Iterator& iter)            = default;
        Iterator(Iterator&& iter)                 = default;
        Iterator& operator=(const Iterator& iter) = default;
        Iterator& operator=(Iterator&& iter)      = default;

        ~Iterator() = default;

        std::int32_t operator*() const noexcept;

        Iterator& operator++() noexcept;

        Iterator operator++(int) noexcept;

        friend bool operator==(const Iterator& a, const Iterator& b);
        friend bool operator!=(const Iterator& a, const Iterator& b);

    private:
        /** The currently referenced element */
        std::int32_t* ptr_;
    };

    MoveList();

    MoveList(const MoveList& list)            = default;
    MoveList(MoveList&& list)                 = default;
    MoveList& operator=(const MoveList& list) = default;
    MoveList& operator=(MoveList&& list)      = default;

    ~MoveList() = default;

    void Append(std::int32_t move) noexcept;
    void Clear() noexcept;

    Iterator Begin() noexcept;
    Iterator End()   noexcept;

private:
    /** The fixed-size list of moves */
    std::array<std::int32_t, 256>
        moves_;

    /** Number of moves in this list */
    std::size_t size_;
};

}  // namespace chess

#endif  // CHESS_MOVELIST_H_
