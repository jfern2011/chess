#ifndef __SELECTION_SORT2_H__
#define __SELECTION_SORT2_H__

#include <utility>

#include "DataTables4.h"
#include "src/chess_util4.h"

namespace Chess
{
	/**
	 * A convenience class used to sort a list of moves
	 *
	 * @tparam Func The type of functor used for comparing moves
	 *
	 * @details
	 * Each call to \ref next() scans the input array for the next
	 * best move and returns it. This avoids sorting the entire list
	 * up front, since usually we only end up searching the first
	 * few best moves
	 */
	template <class Func>
	class SelectionSort2
	{

	public:

		/**
		 * Constructor
		 *
		 * @param[in] moves The list of moves to sort
		 * @param[in] size  The number of moves in \a moves
		 * @param[in] fn    The move comparison functor
		 */
		SelectionSort2(int32* moves, size_t size, Func fn)
			: _func(fn),
			  _iter(0),
			  _moves(moves),
			  _size(size)
		{
		}

		/**
		 * Destructor
		 */
		~SelectionSort2()
		{
		}

		/**
		 * Get the next move in the sorted list. If the list
		 * is exhausted, outputs the last element
		 *
		 * @param[out] move The next best move
		 *
		 * @return True if \a move is valid, or false if the
		 *         list is exhausted
		 */
		bool next(int32& move)
		{
			if (_size <= _iter) return false;

			size_t max_i = _iter;

			for (size_t j = _iter+1; j < _size; ++j)
			{
				if ( _func(_moves[j], _moves[max_i])
					> 0) max_i = j;
			}

			move = _moves[max_i];

			std::swap(_moves[ max_i ],
				_moves[_iter++]);

			return true;
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

	private:

		/**
		 * Function object that compares moves
		 */
		Func _func;

		/**
		 *  The current "outer loop" iteration
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
