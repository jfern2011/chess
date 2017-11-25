#include "DataTables.h"

DataTables tables;

int main()
{
	std::cout << "Running test...";
	std::fflush(stdout);

	if (tables.run_test())
		std::cout << "passed." << std::endl;
	else
		std::cout << "failed." << std::endl;
	
	return 0;
}
