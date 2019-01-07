#include <iterator>

#include "Variation.h"

namespace Chess
{
	/**
	 * Constructor
	 */
	Variation::Variation()
		: _in_use(0), _lines()
	{
	}

	/**
	 * Destructor
	 */
	Variation::~Variation()
	{
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
		auto iter = _lines.begin();

		AbortIf(index >= _in_use, iter->line);

		std::advance(iter, index);

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
		const bool space_left = _in_use < _lines.size();

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
			_in_use++;

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
		else
			_in_use++;

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
		if (size < _in_use) _in_use = size;

		_lines.resize(size);
	}

	/**
	 * Get the number of lines currently stored
	 *
	 * @return The number of lines
	 */
	size_t Variation::size() const
	{
		return _in_use;
	}
}
