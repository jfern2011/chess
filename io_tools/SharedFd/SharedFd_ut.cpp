#include "abort.h"
#include "SharedFd.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <utility>

class SharedFd_ut
{

public:

	SharedFd_ut() {}

	~SharedFd_ut() {}

	bool check_use_count(int fd, int expected)
	{
		return SharedFd::_use_count[fd] == expected;
	}

	bool check_use_count(int start, int stop, int expected)
	{
		for (int i = start; i < stop; i++)
		{
			if (SharedFd::_use_count[i] != expected)
			{
				std::printf("[error]: _use_count[%d] = %d, expected %d\n",
							i, SharedFd::_use_count[i], expected);
				return false;
			}
		}

		return true;
	}

	bool run_test()
	{
		SharedFd fd;
		AbortIfNot(check_use_count(0, FD_SETSIZE, 0), false);

		SharedFd fd1(STDOUT_FILENO);
		AbortIfNot(check_use_count(STDOUT_FILENO, 1), false);

		SharedFd fd2(STDERR_FILENO);
		AbortIfNot(check_use_count(STDERR_FILENO, 1), false);

		SharedFd fd1_cp(fd1);
		AbortIfNot(check_use_count(STDOUT_FILENO, 2), false);

		SharedFd fd1_mv(std::move(fd1));
		AbortIfNot(check_use_count(STDOUT_FILENO, 2), false);

		AbortIf(fd1, false);

		SharedFd fd2_mv = std::move(fd2);
		AbortIfNot(check_use_count(STDERR_FILENO, 1), false);

		AbortIf(fd2, false);

		SharedFd fd2_cp = fd2_mv;
		AbortIfNot(check_use_count(STDERR_FILENO, 2), false);

		fd = fd1_cp;
		AbortIfNot(fd, false);
		AbortIfNot(check_use_count(STDOUT_FILENO, 3), false);

		AbortIfNot(check_use_count(STDERR_FILENO, 2), false);
		fd2_cp.reset();
		AbortIfNot(check_use_count(STDERR_FILENO, 1), false);
		fd2_mv.reset();
		AbortIfNot(check_use_count(STDERR_FILENO, 0), false);

		fd2.reset();
		AbortIfNot(check_use_count(STDERR_FILENO, 0), false);
		fd2_cp.reset();
		AbortIfNot(check_use_count(STDERR_FILENO, 0), false);
		fd2_mv.reset();
		AbortIfNot(check_use_count(STDERR_FILENO, 0), false);

		SharedFd fd3(STDIN_FILENO);
		AbortIfNot(check_use_count(STDIN_FILENO, 1), false);
		SharedFd fd3_cp(fd3);
		AbortIfNot(check_use_count(STDIN_FILENO, 2), false);

		fd3.reset(STDOUT_FILENO);
		AbortIfNot(check_use_count(STDIN_FILENO , 1), false);
		AbortIfNot(check_use_count(STDOUT_FILENO, 4), false);

		fd3_cp.reset(STDOUT_FILENO);
		AbortIfNot(check_use_count(STDIN_FILENO , 0), false);
		AbortIfNot(check_use_count(STDOUT_FILENO, 5), false);

		fd3_cp.reset(STDOUT_FILENO);
		AbortIfNot(check_use_count(STDIN_FILENO , 0), false);
		AbortIfNot(check_use_count(STDOUT_FILENO, 5), false);

		AbortIfNot(fd, false);
		AbortIfNot(fd1_cp, false);
		AbortIfNot(fd1_mv, false);
		AbortIfNot(fd3, false);
		AbortIfNot(fd3_cp, false);

		fd.reset(STDIN_FILENO);
		fd1_cp.reset(STDIN_FILENO);
		fd1_mv.reset(STDIN_FILENO);

		fd3.reset(STDIN_FILENO);
		fd3_cp.reset(STDIN_FILENO);

		AbortIfNot(check_use_count(STDIN_FILENO , 5), false);
		AbortIfNot(check_use_count(STDOUT_FILENO, 0), false);

		AbortIfNot(fd3.use_count() == 5, false);
		AbortIfNot(fd1.use_count() == 0, false);

		fd.reset();
		fd1_cp.reset();
		fd1_mv.reset();
		fd3.reset();

		AbortIfNot(fd3_cp.unique(), false);

		fd3_cp.reset();

		fd1.reset(STDERR_FILENO);
		fd2.reset(STDOUT_FILENO);

		AbortIfNot(fd1.get() == STDERR_FILENO, false);
		AbortIfNot(fd2.get() == STDOUT_FILENO, false);

		AbortIfNot(check_use_count(STDERR_FILENO, 1), false);
		AbortIfNot(check_use_count(STDOUT_FILENO, 1), false);

		fd2.swap(fd1);

		AbortIfNot(fd1.get() == STDOUT_FILENO, false);
		AbortIfNot(fd2.get() == STDERR_FILENO, false);

		AbortIfNot(check_use_count(STDERR_FILENO, 1), false);
		AbortIfNot(check_use_count(STDOUT_FILENO, 1), false);

		fd1.reset();
		fd2.reset();

		return true;
	}

	bool run()
	{
		AbortIfNot(run_test(), false);
		AbortIfNot(check_use_count(0, FD_SETSIZE, 0), false);

		return true;
	}
};

int main()
{
	std::printf("FD_SETSIZE = %d\n", FD_SETSIZE);

	SharedFd_ut ut;
	AbortIfNot(ut.run(), EXIT_FAILURE);

	std::printf("done.\n"); std::fflush(stdout);
	return EXIT_SUCCESS;
}
