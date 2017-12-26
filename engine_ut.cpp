#include "engine.h"

DataTables tables;

int main(void)
{
	ChessEngine engine(tables);

	if (!engine.init(STDIN_FILENO, STDERR_FILENO,
		uci_protocol))
	{
		std::cout << "Error..." << std::endl;
		return 0;
	}

	if (!engine.run(pvs))
		std::cout << "Runtime error..." << std::endl;

	return 0;
}
