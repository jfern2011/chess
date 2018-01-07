#include "CommandLine.h"
#include "engine.h"

#include <cstdlib>
#include <fcntl.h>

/**
 * The global databases used throughout the engine
 */
DataTables tables;

/**
 * Create command line options here
 *
 * @param[in] opts The container that options will be added
 *                 to
 *
 * @return True on success
 */
bool create_cmdline_opts(CommandLineOptions& opts)
{
	AbortIfNot(opts.add<std::string>("logpath", "engine.log",
		       "Path to the chess engine log file."),
		false);

	AbortIfNot(opts.add<bool>("help", false,
		       "Print this help message."),
		false);

	return true;
}

/**
 * Parse the command line arguments and start the chess
 * engine
 *
 * @param[in] argc The number of command line arguments
 * @param[in] argv The arguments themselves
 *
 * @return True on success
 */
bool go(int argc, char** argv)
{
	CommandLineOptions options;

	AbortIfNot( create_cmdline_opts(options),
		false);

	CommandLine cmd(options);
	AbortIfNot(cmd.parse(argc, argv), false);

	std::string logpath;
	bool help;

	AbortIfNot(cmd.get("help", help), false);
	if (help)
	{
		options.print(argv[0]);
		return true;
	}
		
	AbortIfNot(cmd.get( "logpath", logpath ),
		false);

	const int logfd =
			::open(logpath.c_str(),O_CREAT | O_RDWR,
				   S_IRUSR | S_IWUSR);
	
	AbortIf(logfd < 0, false);

	ChessEngine engine(tables);

	AbortIfNot(engine.init(pvs, STDIN_FILENO, logfd,
		uci_protocol), false);

	AbortIfNot(engine.run(), false);

	::close(logfd);
	return true;
	
}

/**
 * The program's entry point
 *
 * @param[in] argc The number of command line arguments
 * @param[in] argv The arguments themselves
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char** argv)
{
	AbortIfNot(go(argc, argv), EXIT_FAILURE);
	return EXIT_SUCCESS;
}
