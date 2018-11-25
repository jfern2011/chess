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
		 * Compare two moves. Moves are scored by taking the
		 * difference between the piece captured and the piece moved,
		 * so a positive value indicates the move gains material
		 *
		 * @param[in] move1 The first move
		 * @param[in] move2 The second move
		 *
		 * @return The difference between the move scores.
		 *         A positive value indicates \a move1 is better
		 */
		static int compare(int32 move1, int32 move2)
		{
			const auto& tables = DataTables::get();

			const int gain1 =
				tables.piece_value[extract_captured(move1)] -
				tables.piece_value[extract_moved(move1)];

			const int gain2 =
				tables.piece_value[extract_captured(move2)] -
				tables.piece_value[extract_moved(move2)];

			return gain1 - gain2;
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
