#include "protocol2.h"

int main(int argc, char** argv)
{
	Logger logger;

	std::string dummy;
	UCI uci(logger);

	uci.init(STDIN_FILENO);
	uci.uci(dummy);

	return 0;
}
