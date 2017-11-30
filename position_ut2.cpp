#include <cstdlib>

#include "abort.h"
#include "position2.h"

int main(int argc, char** argv)
{
	DataTables tables;
	Position pos(tables, false);

	AbortIfNot(pos.make_move(0),
		EXIT_FAILURE);

	return EXIT_SUCCESS;
}
