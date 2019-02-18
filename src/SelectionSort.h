#ifndef __SELECTION_SORT_H__
#define __SELECTION_SORT_H__

#include <utility>

#include "chess4.h"

namespace Chess
{
	/**
	 * A convenience class used to sort a list of moves
	 *
	 * @details
	 * Each call to \ref next() scans the input array for the next
	 * best move and returns it. This avoids sorting the entire list
	 * up front, since usually we only end up searching the first
	 * few best moves
	 */
	struct SelectionSort
	{
		/**
		 * The current "outer loop" iteration
		 */
		size_t iter;

		/**  The list of moves */
		int32* moves;

		/**
		 *  Total number of moves in the list
		 */
		size_t size;

		/**
		 * Check if the sorted move list is exhausted
		 *
		 * @return True if exhausted
		 */
		bool empty() const
		{
			return size <= iter;
		}

		/**
		 * Initialize
		 *
		 * @param[in] _moves The list of moves to sort
		 * @param[in] _size  Number of moves in this list
		 */
		void init(int32* _moves, size_t _size)
		{
			moves = _moves; size = _size; iter = 0;
		}

		/**
		 * Get the next move in the sorted list. If the list
		 * is exhausted, \a move is unmodified
		 *
		 * @param[out] move    The next best move
		 * @param[in]  compare Comparison function object
		 *
		 * @return True if \a move is valid, or false if the
		 *         list is exhausted
		 */
		template <class Func>
		bool next(int32& move, Func compare)
		{
			if (empty()) return false;

			size_t max_i = iter;

			for (size_t j = iter+1 ; j < size; ++j)
			{
				if (compare(moves[j], moves[max_i])
					> 0) max_i = j;
			}

			move = moves[max_i];

			std::swap(moves[ max_i ],
				moves[iter++]);

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
