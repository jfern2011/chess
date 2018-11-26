#include <limits>

#include "search.h"

namespace Chess
{
	Search::Search()
		: _is_init(false),
		  _iteration_depth(0),
		  _node_count(0),
		  _position(),
		  _qnode_count(0)
	{
	}

	Search::~Search()
	{
	}

	bool Search::init(Handle<Position> pos)
	{
		if (!pos) return false;

		_position = pos;
		_is_init = true;

		return true;
	}

	int16 Search::run(int timeout, int depth, int32 best)
	{
		if (!_is_init)
		{
			return std::numeric_limits<int16>::max();
		}

		return 0;
	}
}
