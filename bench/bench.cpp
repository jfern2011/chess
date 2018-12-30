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

	std::printf("score = %d, %s\n",
		score, search.get_pv().c_str());
	std::fflush(stdout);

	return EXIT_SUCCESS;
}
