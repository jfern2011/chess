#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include "abort/abort.h"
#include "src/chess4.h"
#include "src/FdChannel.h"
#include "src/search.h"

int main(int argc, char** argv)
{
	Chess::Handle<std::ostream>
			stream(new std::ostream(std::cout.rdbuf()));

	Chess::Handle<Chess::Position>
		pos(new Chess::Position(stream/*, "r6k/6pp/7N/8/8/1Q6/8/7K w - - 0 1"*/));

	Chess::Handle<Chess::OutputChannel> channel(
		new Chess::FdChannel(Fd(STDOUT_FILENO)));

	Chess::Search search(channel);
	AbortIfNot(search.init(pos), EXIT_FAILURE);

	search.enable_multipv(false);

	const size_t numel = 8388608;
	AbortIfNot(search.hash_table.resize(numel), false);

	std::cout << "Hash table = ";
	const size_t table_size = search.hash_table.size() * sizeof(Chess::HashBucket<4>);
	if (table_size > 1e6)
		std::cout << int(table_size / 1e6) << " MB" << std::endl;
	else if (table_size > 1e3)
		std::cout << int(table_size / 1e3) << " KB" << std::endl;
	else
		std::cout << table_size << " bytes" << std::endl;
	std::cout << std::endl;

	search.hash_table.clear();
	search.run(60000, 9, 0);

	/*
	for (size_t i = 0; i < search.lines.size(); i++)
	{
		Chess::int16 score;
		auto pv = search.lines.get(i, score);

		Chess::Position temp(*pos);
		std::string pv_s = Chess::Variation::format(pv, temp);

		std::printf("score = %5d, %s\n", score, pv_s.c_str());
		std::fflush(stdout);
	}*/

	return EXIT_SUCCESS;
}
