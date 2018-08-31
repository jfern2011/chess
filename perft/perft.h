#ifndef __PERFT_H__
#define __PERFT_H__

#include "src/Position4.h"

namespace Chess
{
	int64 divide(Position& pos, int depth);
	int64 perft (Position& pos, int depth);
}

#endif
