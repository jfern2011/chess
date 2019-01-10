#ifndef __PRINCIPAL_VARIATION_H__
#define __PRINCIPAL_VARIATION_H__

#include <list>
#include <string>
#include <vector>

#include "MoveList.h"

namespace Chess
{
	/**
	 * Collects a set of variations (lines) and sorts them
	 * according to score. See \ref insert()
	 */
	class Variation
	{

	public:

		Variation();

		~Variation();

		const std::vector<int32>& operator[](size_t index) const;

		const std::vector<int32>&
			get(size_t index, int16& score) const;

		void clear();

		bool insert(const MoveList& line, int16 score);

		void resize(size_t size);

		size_t size() const;

	private:

		/**
		 * Encapsulates a line with a corresponding score
		 */
		struct ListScore
		{
			/**
			 * Default constructor
			 */
			ListScore() : line(), score(0)
			{
			}

			/**
			 * Constructor
			 *
			 * @param[in] _list  The list of moves to create from
			 * @param[in] _score The corresponding score
			 */
			ListScore(const MoveList& _list, int16 _score)
				: line(), score(_score)
			{
				line.assign(_list.moves, _list.moves +
					_list.size);
			}

			/**
			 * Compare two lines according to their scores
			 *
			 * @param [in] rhs The line to compare ours against
			 *
			 * @return True if our line is better than \a rhs
			 */
			bool operator>(const ListScore& rhs)
			{
				return score > rhs.score;
			}

			/** The line (variation) */
			std::vector<int32> line;

			/** The associated score */
			int16 score;
		};

		/**
		 * The max number of lines allowed
		 */
		size_t _capacity;

		/**
		 * The set of lines saved
		 */
		std::list<ListScore>
			_lines;
	};
}

#endif
