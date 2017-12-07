#include "protocol2.h"

int main(int argc, char** argv)
{
	std::string dummy;
	UCI uci;

	uci.init(STDIN_FILENO);
	uci.uci(dummy);

	return 0;
}
