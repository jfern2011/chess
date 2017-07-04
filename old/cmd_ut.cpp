#include "cmd.h"

#include <iostream>
#include <string>

class TestClass
{
public:
	int testFunc1(const std::string& sentence)
	{
		std::cout << "testFunc1(): " << sentence
			<< std::endl;
		return 0;
	}

	void testFunc2()
	{
		std::cout << "hello from testFunc2()!"
			<< std::endl;
	}
};

int testFunc3(int a)
{
	std::cout << "testFunc3(): a = " << a << std::endl;
	return a;
}

int testFunc4()
{
	std::cout << "hello from testFunc4()!" << std::endl;
	return 0;
}

void testFunc5()
{
	std::cout << "hello from testFunc5()!" << std::endl;
}

void test_suite1()
{
	CommandRouter router;

	TestClass tc;

	int id = router.install("cmd1", tc, &TestClass::testFunc1);

	std::cout << "Installed cmd1 with id "
		<< id
		<< std::endl;

	id = router.install("cmd2", tc, &TestClass::testFunc2);

	std::cout << "Installed cmd2 with id "
		<< id
		<< std::endl;

	id = router.install("cmd3", &testFunc3);

	std::cout << "Installed cmd3 with id "
		<< id
		<< std::endl;

	if (router.getID("cmd1") != 0)
	{
		std::cout << "failed test 1" << std::endl;
	}

	if (router.getID("cmd2") != 1)
	{
		std::cout << "failed test 2" << std::endl;
	}

	if (router.getID("cmd3") != 2)
	{
		std::cout << "failed test 3" << std::endl;
	}

	if (!router.isInstalled("cmd1"))
	{
		std::cout << "failed test 4" << std::endl;
	}

	if (!router.isInstalled("cmd2"))
	{
		std::cout << "failed test 5" << std::endl;
	}

	if (!router.isInstalled("cmd3"))
	{
		std::cout << "failed test 6" << std::endl;
	}

	if (router.isInstalled("cmd4"))
	{
		std::cout << "failed test 7" << std::endl;
	}

	if (!router.isInstalled(0))
	{
		std::cout << "failed test 8" << std::endl;
	}

	if (!router.isInstalled(1))
	{
		std::cout << "failed test 9" << std::endl;
	}

	if (!router.isInstalled(2))
	{
		std::cout << "failed test 10" << std::endl;
	}

	if (router.isInstalled(3))
	{
		std::cout << "failed test 11" << std::endl;
	}

	//router.forward<const std::string&>("cmd1", "handling cmd1...");
	//router.forward<void>("cmd2");
	router.forward<int>("cmd3", 5);

	if (!router.uninstall(0))
	{
		std::cout << "failed test 12" << std::endl;
	}

	if (router.uninstall("cmd1"))
	{
		std::cout << "failed test 13" << std::endl;
	}

	if (!router.uninstall("cmd3"))
	{
		std::cout << "failed test 14" << std::endl;
	}

	id = router.install("cmd4", &testFunc4);

	std::cout << "Installed cmd4 with id "
		<< id
		<< std::endl;

	id = router.install("cmd5", *testFunc5);

	std::cout << "Installed cmd5 with id "
		<< id
		<< std::endl;

	id = router.install("cmd6", tc, &TestClass::testFunc1);

	std::cout << "Installed cmd6 with id "
		<< id
		<< std::endl;

	//router.forward<void>("cmd2");
	//router.forward("cmd4");
	//router.forward<void>("cmd5");
/*
	router.forward(const std::string&>("cmd6",
									"handling cmd6..."));
*/
}

int main()
{
	test_suite1();
	return 0;
}