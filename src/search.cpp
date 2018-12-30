#include <limits>

#include "search.h"

namespace Chess
{
	Search::Search() : _is_init(false), _position()
	{
		_set_defaults();
	}

	Search::~Search()
	{
	}

	std::string Search::get_pv() const
	{
		std::string out("pv ---> ");

		for (size_t i = 0; _pv[0][i] && i < max_ply; i++)
		{
			out += format_san(_pv[0][i], "") + " ";
		}

		return out;
	}

	bool Search::init(Handle<Position> pos)
	{
		if (!pos) return false;

		_position = pos;

		_set_defaults();

		_is_init = true;
		return true;
	}

	int16 Search::run(int timeout, int depth, int32 best)
	{
		if (!_is_init)
		{
			return std::numeric_limits<int16>::max();
		}

		_set_defaults(); _is_init = false;

		return search( depth, -king_value, king_value );
	}

	void Search::_set_defaults()
	{
		_iteration_depth = 3;
		_node_count      = 0;
		_qnode_count     = 0;

		for (size_t i = 0; i < max_ply; i++)
			_pv[0][i] = 0;
	}
}
