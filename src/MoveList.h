#ifndef __MOVE_LIST_H__
#define __MOVE_LIST_H__

#include <utility>

#include "chess4.h"

namespace Chess
{
    /**
     * A convenience class which steps through an unsorted
     * move list
     *
     * @todo Consider implementing as a linked list, to allow
     *       popping searched moves in constant time
     */
    struct MoveList
    {
        /**
         * Index of the last move returned
         */
        int index;

        /**  The list of moves */
        int32* moves;

        /**
         *  Total number of moves in the list
         */
        int size;

        /**
         * Check if the list of moves is exhausted
         *
         * @return True if exhausted
         */
        bool empty() const
        {
            return size <= index;
        }

        /**
         * Search for a particular move in the list
         *
         * @param[in] move Search for this move
         *
         * @return The index if found, -1 otherwise
         */
        int find(int32 move)
        {
            for (int i = 0; i < size; i++)
                if (moves[i] == move) return i;
            return -1;
        }

        /**
         * Initialize
         *
         * @param[in] _moves The list of moves
         * @param[in] _size  Number of moves in the list
         */
        void init(int32* _moves, int _size)
        {
            moves = _moves; size = _size; index = -1;
        }

        /**
         * Get the next move in the list. If the list is
         * exhausted, \a move is unmodified
         *
         * @param[out] move The next move
         *
         * @return True if \a move is valid, or false if
         *         the list is exhausted
         */
        bool next(int32& move)
        {
            ++index;
            if (empty()) return false;

            move = moves[index];
            return true;
        }

        /**
         * Add a move to the end of the list
         */
        void push_back(int32 move)
        {
            moves[size++] = move;
        }
    };
}

#endif
