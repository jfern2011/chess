#ifndef __SELECTION_SORT_H__
#define __SELECTION_SORT_H__

#include <utility>

#include "DataTables4.h"
#include "src/chess_util4.h"

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
	class SelectionSort
	{

	public:

		/**
		 * Constructor
		 *
		 * @param[in] moves The list of moves to sort
		 * @param[in] size  The number of moves in \a moves
		 */
		SelectionSort(int32* moves, size_t size)
			: _iter(0),
			  _moves(moves),
			  _size(size)
		{
		}

		/**
		 * Destructor
		 */
		~SelectionSort()
		{
		}

		/**
		 * Get the next move in the sorted list. If the list
		 * is exhausted, returns the last element
		 *
		 * @return The next best move
		 */
		int32 next()
		{
			size_t max_i = _iter;

			for (size_t j = _iter+1; j < _size; ++j)
			{
				if (compare(_moves[j], _moves[max_i]) > 0)
					max_i = j;
			}

			const int32 best = _moves[max_i];

			if (_iter < _size-1)
				std::swap(_moves[max_i], _moves[_iter++]);

			return best;
		}

		/**
		 * Get the number of moves in the array
		 *
		 * @return The number of elements
		 */
		size_t size() const
		{
			return _size;
		}

		/**
		 * Compare two moves
		 *
		 * @note See \ref score() for a description of how moves are
		 *       scored
		 *
		 * @param[in] move1  A 21-bit packed move
		 * @param[in] move2  The second move against which to compare
		 *                   \a move1
		 *
		 * @return The difference between the two move scores; if
		 *         positive, \a move1 is better
		 */
		static int compare(int32 move1, int32 move2)
		{
			return score(move1) - score(move2);
		}

		/**
		 * Score a move as the result of the difference in value
		 * between the piece captured and the piece moved. A positive
		 * value indicates that playing this move gains material
		 *
		 * @param[in] move The move to score
		 *
		 * @return The score
		 */
		static int score(int32 move)
		{
			return DataTables::get().exchange[extract_captured(
				move)][extract_moved(move)];
		}

	private:

		/**
		 * The current "outer loop" iteration
		 */
		size_t _iter;

		/**  The list of moves */
		int32* const _moves;

		/**
		 * Number of moves in the list
		 */
		const size_t _size;
	};
}

#endif
