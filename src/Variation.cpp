#include <iterator>

#include "Variation.h"

namespace Chess
{
	/**
	 * Constructor
	 */
	Variation::Variation()
		: _capacity(0), _lines()
	{
	}

	/**
	 * Destructor
	 */
	Variation::~Variation()
	{
	}

	/**
	 * Clear all saved variations
	 */
	void Variation::clear()
	{
		_lines.clear();
	}

	/**
	 * Get the line at the specified index. The best line is at
	 * index 0, the 2nd best is at index 1, and so on
	 *
	 * @param[in] index The index of the line to retrieve
	 *
	 * @return  The line. The behavior is undefined if \a index
	 *          is out of bounds
	 */
	const std::vector<int32>& Variation::operator[](
		size_t index) const
	{
		int16 dummy; return get(index, dummy);
	}

	/**
	 * Get the line at the specified index. The best line is at
	 * index 0, the 2nd best is at index 1, and so on
	 *
	 * @param[in]  index The index of the line to retrieve
	 * @param[out] score The corresponding score
	 *
	 * @return  The line. The behavior is undefined if \a index
	 *          is out of bounds
	 */
	const std::vector<int32>& Variation::get(
		size_t index, int16& score) const
	{
		auto iter = _lines.begin();

		AbortIf(index >= _lines.size(), iter->line);

		std::advance(iter, index);

		score = iter->score;
			return iter->line;
	}

	/**
	 * Insert a new line. Lines are sorted in descending order,
	 * starting with the best line
	 *
	 * @param [in] line   The line to insert
	 * @param [in] score  The corresponding score for this line
	 *
	 * @return True if we successfully inserted \a line
	 */
	bool Variation::insert(const MoveList& line, int16 score)
	{
		const bool space_left = _lines.size() < _capacity;

		bool inserted = false;
		for (auto iter  =  _lines.begin(), end = _lines.end();
			 iter != end; ++iter)
		{
			if (score > iter->score)
			{
				_lines.emplace( iter, line, score );
				inserted = true;
				break;
			}
		}

		/*
		 * If this line is worse than all others and there is
		 * no space left, don't do anything
		 */
		if (!inserted && !space_left)
			return false;

		/*
		 * If this line is worse than all others and there is
		 * space left, place it at the end of the list
		 */
		if (!inserted && space_left)
		{
			_lines.push_back(std::move(ListScore(
				line, score)));

			return true;
		}

		/*
		 * We inserted the new line since it was better than
		 * at least one of the others. However, if we exceeded
		 * the limit set by resize(), then delete the last
		 * (i.e. worst) line
		 */
		if (!space_left)
			_lines.pop_back();

		return true;
	}

	/**
	 * Reset the limit on the number of lines that are stored,
	 * preserving all entries up to \a size
	 *
	 * @param[in] size The new size
	 */
	void Variation::resize(size_t size)
	{
		_capacity = size;

		if ( _capacity < _lines.size() )
			_lines.resize(_capacity);
	}

	/**
	 * Get the number of lines currently stored
	 *
	 * @return The number of lines
	 */
	size_t Variation::size() const
	{
		return _lines.size();
	}
}
