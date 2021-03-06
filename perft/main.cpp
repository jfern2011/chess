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
	AbortIfNot(options.add<int>("depth", 1, "Max depth, in plies"),
		false);

	AbortIfNot(options.add<std::string>("fen",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
		"The FEN position"), false);

	AbortIfNot(options.add<bool>("divide", false,
		"Show the number of nodes per move"), false);

	AbortIfNot(options.add<bool>("help", false,
		"Print this help message"), false);

	AbortIfNot(options.add<bool>("gen_checks", false,
			"Test the checking moves generator"),
		false);

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

	bool divide = false;
	AbortIfNot(cmd.get("divide", divide),
		false);

	Chess::Handle<std::ostream> stream(
		new std::ostream( std::cout.rdbuf()) );

	Chess::Position pos(stream);
	AbortIfNot(pos.reset(fen), false);

	Chess::Timer timer;

	std::int64_t nodes;

	bool do_checks;
	AbortIfNot(cmd.get("gen_checks", do_checks),
		false);

	if (divide)
	{
		if (do_checks)
		{
			std::cout << "'divide' option does not generate checks"
				<< std::endl;
		}

		timer.start();
		nodes = Chess::divide(pos, depth);
		timer.stop();
	}
	else
	{
		timer.start();
		if (do_checks)
			nodes = Chess::perft_checks(pos, depth);
		else
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
	AbortIfNot(run(argc, argv), EXIT_FAILURE);
	return EXIT_SUCCESS;
}
