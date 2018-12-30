#ifndef __EVAL_H__
#define __EVAL_H__

#include "Position4.h"

namespace Chess
{
	template <player_t P>
	int evaluate_pawns(const Position& pos)
	{
		return 0;
	}

	inline int evaluate_mobility(const Position& pos)
	{
		return 0;
	}

	inline int evaluate(const Position& pos)
	{
		return pos.get_material(player_t::white)
				- pos.get_material( player_t::black );
	}
}

#endif
