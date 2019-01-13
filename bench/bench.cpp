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

	search.enable_multipv(true);

	search.run(0, 0, 0);

	for (size_t i = 0; i < search.lines.size(); i++)
	{
		Chess::int16 score;
		auto pv = search.lines.get(i, score);

		Chess::Position temp(*pos);
		std::string pv_s = Chess::Variation::format(pv, temp);

		std::printf("score = %5d, %s\n", score, pv_s.c_str());
		std::fflush(stdout);
	}

	return EXIT_SUCCESS;
}
