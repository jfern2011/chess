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

	AbortIfNot(opts.add<std::string>("protocol", "none",
			   "The communication protocol to use."),
		false);

	return true;
}

/**
 * Get the protocol to use. This step is done prior to
 * initialization
 *
 * @return The protocol enumerated value
 */
protocol_t get_protocol(const CommandLine& cmd)
{
	/*
	 * First, check if the protocol was set via
	 * the command line:
	 */
	std::string type;
	AbortIfNot(cmd.get("protocol", type),
		none);

	if (type == "none")
	{
		/*
		 * The protocol was not specified, so try getting it
		 * from standard input:
		 */
		Buffer<char,1024> buf;

		AbortIf(::read(STDIN_FILENO, buf, 1024) < 0,
			none);

		Util::str_v input;
		Util::split((char*)buf, input, '\n');

		AbortIf(input.empty(), none);

		type =
			Util::to_lower ( Util::trim(input[0]) );
	}

	if (type == "xboard")
		return xboard_protocol;
	if (type == "uci")
		return uci_protocol;
	if (type == "console")
		return console_mode;

	return none;
}

/**
 * If a file exists, return a unique name for it; otherwise,
 * return the file name
 *
 * @param[in]  name     The original name of the file
 * @param[out] new_name A possibly new file name
 *
 * @return True on success
 */
bool get_unique_filename(const std::string& name,
	std::string& new_name)
{
	new_name = name;

	int suffix = 1;
	while (Util::dirExists(new_name))
	{
		std::string str;
		AbortIfNot( Util::to_string(suffix, str),
			false);

		new_name = name + "." + str;
		suffix++;
	}

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

	std::string _logpath;
	bool help;

	AbortIfNot(cmd.get("help", help), false);
	if (help)
	{
		options.print(argv[0]);
		return true;
	}
		
	AbortIfNot(cmd.get( "logpath", _logpath),
		false);

	protocol_t protocol_enum =
		get_protocol(cmd);
		
	AbortIf(protocol_enum == none,
		false);

	std::string logpath;
	AbortIfNot(get_unique_filename(_logpath, logpath),
		false);

	const int logfd =
		::open(logpath.c_str(), O_CREAT | O_RDWR);
	
	AbortIf(logfd < 0, false);

	ChessEngine engine(tables);

	AbortIfNot(engine.init(pvs, STDIN_FILENO, logfd,
		protocol_enum), false);

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
