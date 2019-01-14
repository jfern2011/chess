#include "abort.h"
#include "FdSet.h"

#include <cstdlib>
#include <fcntl.h>

#include <sys/epoll.h>
#include <unistd.h>

bool quit_request = false;

bool write_handler(const SharedFd& fd)
{
	std::printf("%s: Got fd %d.\n", __FUNCTION__, fd.get());
	std::fflush(stdout);
	return true;
}

bool read_handler(const SharedFd& fd)
{
	char buf[1024];

	int n = ::read(fd.get(), buf, 1024);
	AbortIf(n < 0, false);

	int end= buf[n-1] == '\n' ? n-1 : n;

	buf[end] = '\0';
	std::string input(buf);

	std::printf("%s: Got fd %d: '%s' (%d bytes).\n",
				__FUNCTION__, fd.get(), buf, end);
	std::fflush(stdout);

	if (input.find("quit") != std::string::npos)
		quit_request = true;

	return true;
}

bool except_handler(const SharedFd& fd)
{
	std::printf("%s: Got fd %d.\n", __FUNCTION__, fd.get());
	std::fflush(stdout);
	return true;
}

int main()
{
	FdSet fds;

	AbortIfNot(fds.push_back(SharedFd(STDIN_FILENO), EPOLLIN),
		EXIT_FAILURE);

	AbortIfNot(fds.read_sig.attach(&read_handler),
		false);

	AbortIfNot(fds.except_sig.attach(&except_handler),
		false);

	while (!quit_request)
	{
		AbortIfNot(fds.poll(0), false);
		::sleep(1);

		std::printf("no data...\n");
		std::fflush(stdout);
	}

	std::printf("tests complete.\n");
	std::fflush(stdout);

	return 0;
}
