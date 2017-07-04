#include "DataTables.h"

DataTables tables;

int main()
{
	std::cout << "Running test...";
	if (tables.run_test())
		std::cout << "passed." << std::endl;
	return 0;
}