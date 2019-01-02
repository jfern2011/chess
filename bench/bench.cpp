#include <cstdlib>
#include <iostream>

#include "abort/abort.h"
#include "src/chess4.h"
#include "src/search.h"

int main(int argc, char** argv)
{
	Chess::Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

	Chess::Handle<Chess::Position>
		pos(new Chess::Position(stream, "r6k/6pp/7N/8/8/1Q6/8/7K w - - 0 1"));

	Chess::Search search;
	AbortIfNot(search.init(pos), EXIT_FAILURE);

	const Chess::int16 score = search.run(0, 0, 0);

	Chess::MoveList pv = search.get_pv();
	std::string pv_s("pv ---> ");
	for (int i = 0; i < pv.size; i++)
		pv_s += Chess::format_san(pv.moves[i], "") + " ";

	std::printf("score = %d, %s\n", score, pv_s.c_str());
	std::fflush(stdout);

	return EXIT_SUCCESS;
}
