#include <fcntl.h>

#include "engine.h"

DataTables tables;

int main(void)
{
	ChessEngine engine(tables);

	//const int logfd = ::open("log", O_CREAT | O_RDWR);
	const int logfd = STDERR_FILENO;
	AbortIf(logfd < 0, false);

	if (!engine.init(STDIN_FILENO, logfd,
		uci_protocol))
	{
		std::cout << "Error..." << std::endl;
		return 0;
	}

	if (!engine.run(pvs))
		std::cout << "Runtime error..." << std::endl;

	//::close(logfd);
	return 0;
}
