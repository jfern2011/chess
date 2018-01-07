#include <csignal>
#include <cstdio> // STDIN_FILENO
#include <iostream>
#include <Signal.h>
#include <unistd.h>

#include "ReadEventSink.h"

bool exit_flag = false;

void sigint_handler(int signum)
{
	exit_flag = true;
	std::cout << "exiting..." << std::endl;
}

class Test
{
public:
	bool sink(const char* args, size_t size)
	{
		std::cout << "Test::sink(): "  << args << " (" << size
			<< " bytes)" << std::endl;
		return true;
	}

	static bool sink2(const char* args, size_t size)
	{
		std::cout << "Test::sink2(): " << args << " (" << size
			<< " bytes)" << std::endl;
		return true;
	}
};

bool basic_test()
{
	const int fd = STDIN_FILENO;
	ReadEventSink res1;
	ReadEventSink res2(fd);

	AbortIfNot(res1.assign_fd(fd), false);
	AbortIfNot( res1.attach_reader ( & Test::sink2 ), false);

	Test test;
	AbortIfNot(res2.attach_reader(test, &Test::sink), false);

	ReadEventSink res3(std::move(res1));
	ReadEventSink res4(res2);

	ReadEventSink res5(res3);
	ReadEventSink res6(res4);

	res5 = std::move(res3); // Test::sink2()
	res6 = res4;            // Test::sink()

	struct Check
	{
    	static bool is_ok(err_code code)
    	{
    		return code == ReadEventSink::NO_DATA ||
    			   code == ReadEventSink::SUCCESS;
    	}
    };

    AbortIfNot(Check::is_ok(res5.poll()), false);
    AbortIfNot(Check::is_ok(res6.poll()), false);

    std::cout << "> "; std::fflush(stdout);
    // '.' delimited read:
    AbortIfNot(Check::is_ok(res5.read(".", true, -1)), false);

    std::cout << "> "; std::fflush(stdout);
    // 12-byte reads:
    AbortIfNot(Check::is_ok(res6.read(12 , true, -1)), false);

	return true;
}

/**
 * Mode 1: Read by delimiter (newline won't work here)
 * Mode 2: Read by number of bytes
 * Mode 3: Read until delimiter (newline won't work here)
 * Mode 4: Read until number of bytes
 */
int main(int argc, char** argv)
{
	std::string delimiter;
	bool clear;
	int mode;
	size_t nbytes;
	char ans;

	std::cout << "Running test 1..." << std::endl;

	AbortIfNot(basic_test(), false);
	
	std::cout << "Running test 2..." << std::endl;

	std::cout << "Mode: ";
	std::cin >> mode;

	switch (mode)
	{
	case 1:
		std::cout << "Reading by delimiter: ";
		std::cin >> delimiter;
		break;
	case 2:
		std::cout << "Reading by number of bytes: ";
		std::cin >> nbytes;
		break;
	case 3:
		std::cout << "Reading until delimiter: ";
		std::cin >> delimiter;
		break;
	case 4:
		std::cout << "Reading until number of bytes: ";
		std::cin >> nbytes;
		break;
	default:
		std::cout << "Invalid mode: "
			<< mode << std::endl;
		return 0;
	}

	std::cout << "Clear internal buffers prior to reading (y/n)? ";
	std::cin >> ans;

	clear = (ans == 'Y' || ans == 'y');

	int64 timeout;
	std::cout << "Enter a poll timeout (nanoseconds): ";
	std::cin >> timeout;

	AbortIf(timeout < 0, -1);

	::signal(SIGINT, &sigint_handler);

	Test test;
	ReadEventSink event_sink(STDIN_FILENO);
	event_sink.attach_reader(test, &Test::sink);

	do
	{
		switch (mode)
		{
		case 1:
			event_sink.read(delimiter, clear, timeout); break;
		case 2:
			event_sink.read(nbytes, clear, timeout); break;
		case 3:
			event_sink.read_until(delimiter, clear, timeout); break;
		case 4:
			event_sink.read_until(nbytes, clear, timeout);
		}

	} while (!exit_flag);

	return 0;
}
