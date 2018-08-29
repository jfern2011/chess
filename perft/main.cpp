#include <cstdlib>
#include <iostream>

#include "abort/abort.h"
#include "CommandLine/CommandLine.h"
#include "src/chess4.h"
#include "perft.h"

bool init_options(CommandLineOptions& options)
{
	AbortIfNot(options.add<int>("depth", 1, "Max depth, in plies"),
		false);

	AbortIfNot(options.add<std::string>("fen",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"The FEN position"), false);

	AbortIfNot(options.add<bool>("help", false,
		"Print this help message"), false);

	return true;
}

bool run(int argc, char** argv)
{
	CommandLineOptions opts;
	AbortIfNot(init_options(opts),
		false);

	CommandLine cmd(opts);
	AbortIfNot(cmd.parse(argc, argv), false);

	bool help = false;
	AbortIfNot(cmd.get("help", help), false);

	if (help)
	{
		opts.print(argv[0]);
		return true;
	}

	std::string fen;
	AbortIfNot(cmd.get("fen", fen), false);

	int depth;
	AbortIfNot(cmd.get("depth", depth), false);

	Chess::Handle<std::ostream> stream(
		new std::ostream( std::cout.rdbuf()) );

	Chess::Position pos(stream);
	AbortIfNot(pos.reset(fen), false);

	std::int64_t nodes = Chess::perft(pos, depth);

	std::cout << "nodes = " << nodes
		<< std::endl;

	return true;
}

int main(int argc, char** argv)
{
	AbortIfNot(run(argc, argv), EXIT_FAILURE);
	return EXIT_SUCCESS;
}
