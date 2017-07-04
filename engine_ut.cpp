#include "engine.h"

DataTables tables;

int main(void)
{
	ChessEngine engine(tables);

	if (!engine.init())
		std::cout << "Error..." << std::endl;

	if (!engine.run())
		std::cout << "Runtime error..." << std::endl;

	return 0;
}
