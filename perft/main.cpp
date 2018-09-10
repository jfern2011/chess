#include <cstdlib>
#include <iomanip>
#include <iostream>

#include "abort/abort.h"
#include "CommandLine/CommandLine.h"
#include "src/chess4.h"
#include "src/Timer.h"
#include "perft.h"

bool init_options(CommandLineOptions& options)
{
	AbortIfNot_2(options.add<int>("depth",1,"Max depth, in plies"),
		false);

	AbortIfNot_2(options.add<std::string>("fen",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"The FEN position"), false);

	AbortIfNot_2(options.add<bool>("divide", false,
		"Show the number of nodes per move"), false);

	AbortIfNot_2(options.add<bool>("help", false,
		"Print this help message"), false);

	return true;
}

bool run(int argc, char** argv)
{
	CommandLineOptions opts;
	AbortIfNot_2(init_options(opts),
		false);

	CommandLine cmd(opts);
	AbortIfNot_2(cmd.parse(argc, argv), false);

	bool help = false;
	AbortIfNot_2(cmd.get("help", help), false);

	if (help)
	{
		opts.print(argv[0]);
		return true;
	}

	std::string fen;
	AbortIfNot_2(cmd.get("fen", fen), false);

	int depth;
	AbortIfNot_2(cmd.get("depth", depth), false);

	bool divide = false;
	AbortIfNot_2(cmd.get("divide", divide),
		false);

	Chess::Handle<std::ostream> stream(
		new std::ostream( std::cout.rdbuf()) );

	Chess::Position pos(stream);
	AbortIfNot_2(pos.reset(fen), false);

	Chess::Timer timer;

	std::int64_t nodes;

	if (divide)
	{
		timer.start();
		nodes = Chess::divide(pos, depth);
		timer.stop();
	}
	else
	{
		timer.start();
		nodes = Chess::perft (pos, depth);
		timer.stop();
	}

	double dt = timer.elapsed() / 1.0e9;

	std::cout << "nodes = " << nodes << ", time = ";
	std::cout << std::fixed << std::setprecision(6);
	std::cout << dt << "s" << std::endl;

	return true;
}

int main(int argc, char** argv)
{
	AbortIfNot_2(run(argc, argv), EXIT_FAILURE);
	return EXIT_SUCCESS;
}
