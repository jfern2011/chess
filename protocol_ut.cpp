#include "protocol2.h"

int main(int argc, char** argv)
{
	Logger logger;
	logger.assign_fd(STDERR_FILENO);

	std::string dummy;
	UCI uci(logger);

	uci.init(STDIN_FILENO);
	uci.uci(dummy);
	uci.debug("off");
	uci.debug("on");
	uci.isready(dummy);
	uci.setoption("name hash value 10");
	uci.setoption("name ponder value true");

	return 0;
}
