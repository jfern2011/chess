#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "cmd.h"

static bool exit_now = false;

bool echo(const std::string& str)
{
	std::cout << "You entered '" << str << "'"
		<< std::endl;
	return true;
}

bool quit(const std::string& str)
{
	return exit_now = true;
}

bool run()
{
	CommandInterface cmd;

	AbortIfNot(cmd.init(STDIN_FILENO), false);

	AbortIfNot(cmd.install("echo", &echo),
		false);

	AbortIfNot(cmd.install("quit", &quit),
		false);

	while (!exit_now)
	{
		::sleep(1); cmd.poll();
	}

	return true;
}

int main()
{
	AbortIfNot(run(), EXIT_FAILURE);
	return
		EXIT_SUCCESS;
}
