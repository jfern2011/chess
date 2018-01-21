#include <iostream>
#include <string>

#include "abort.h"
#include "Signal.h"

namespace test_funcs
{
	void func1()
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
	}

	void func2(const std::string& str, int a)
	{
		std::cout << __PRETTY_FUNCTION__
			<< ": str = " << str << ", a = " << a
			<< std::endl;
	}

	std::string func3()
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
		return "";
	}

	void func4()
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
	}

	void func5(const std::string& str, int a)
	{
		std::cout << __PRETTY_FUNCTION__
			<< ": str = " << str << ", a = " << a
			<< std::endl;
	}

	std::string func6()
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
		return "";
	}
}

class test_class
{

public:

	void func1()
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
	}

	void func2(const std::string& str, int a)
	{
		std::cout << __PRETTY_FUNCTION__
			<< ": str = " << str << ", a = " << a
			<< std::endl;
	}

	std::string func3() const
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
		return "";
	}

	void func4()
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
	}

	void func5(const std::string& str, int a)
	{
		std::cout << __PRETTY_FUNCTION__
			<< ": str = " << str << ", a = " << a
			<< std::endl;
	}

	std::string func6() const
	{
		std::cout << "Hello from " << __PRETTY_FUNCTION__
			<< "!" << std::endl;
		return "";
	}

	bool operator()(const std::string& str) // Callable
	{
		std::cout << __PRETTY_FUNCTION__
			<< ": str = "
			<< str << std::endl;
		return true;
	}
};

class mem_ptr_test
{

public:

	bool run()
	{
		test_class obj;
		Signal::mem_ptr<void,test_class> mem_ptr1(obj);

		Signal::mem_ptr<void,test_class,const std::string&,int>
			mem_ptr2(obj, &test_class::func2);

		Signal::mem_ptr<std::string,test_class>
			mem_ptr3(obj,&test_class::func3);

		AbortIf(mem_ptr1.is_connected(), false);
		AbortIfNot(mem_ptr2.is_connected(), false);
		AbortIfNot(mem_ptr3.is_connected(), false);

		AbortIfNot(mem_ptr1.attach(&test_class::func1), false);
		AbortIfNot(mem_ptr1.is_connected(), false);

		mem_ptr2.bind("hello", 1);
		mem_ptr2.raise();

		mem_ptr2.raise("sup", 100);

		mem_ptr1.raise();
		mem_ptr3.raise();

		AbortIfNot(mem_ptr1.attach(&test_class::func4), false);
		AbortIfNot(mem_ptr2.attach(&test_class::func5), false);
		AbortIfNot(mem_ptr3.attach(&test_class::func6), false);

		mem_ptr2.bind("hello again!", 2);

		Signal::generic* gen_sig = &mem_ptr2;
		gen_sig->v_raise();

		mem_ptr2.raise("sup yo", 200);

		mem_ptr1.raise();
		mem_ptr3.raise();

		auto sig = dynamic_cast<
			Signal::signal_t<void,const std::string&,int>*
				>(mem_ptr2.clone());

		sig->raise("I'm a mem_ptr clone!", 42);
		auto sig2 = dynamic_cast<
			Signal::mem_ptr<void,test_class,const std::string&,int>*
			>(sig);
		sig2->raise();
		delete sig;

		AbortIfNot(mem_ptr1.detach(), false);
		AbortIfNot(mem_ptr2.detach(), false);
		AbortIfNot(mem_ptr3.detach(), false);
		AbortIf(mem_ptr1.is_connected(), false);
		AbortIf(mem_ptr2.is_connected(), false);
		AbortIf(mem_ptr3.is_connected(), false);

		return true;
	}
};

class fcn_ptr_test
{

public:

	bool run()
	{
		Signal::fcn_ptr<void> fcn_ptr1;

		Signal::fcn_ptr<void,const std::string&,int>
			fcn_ptr2(&test_funcs::func2);

		Signal::fcn_ptr<std::string>
			fcn_ptr3(&test_funcs::func3);

		AbortIf(fcn_ptr1.is_connected(), false);
		AbortIfNot(fcn_ptr2.is_connected(), false);
		AbortIfNot(fcn_ptr3.is_connected(), false);

		AbortIfNot(fcn_ptr1.attach(&test_funcs::func1), false);
		AbortIfNot(fcn_ptr1.is_connected(), false);

		fcn_ptr2.bind("hello", 1);
		fcn_ptr2.raise();

		fcn_ptr2.raise("sup", 100);

		fcn_ptr1.raise();
		fcn_ptr3.raise();

		AbortIfNot(fcn_ptr1.attach(&test_funcs::func4), false);
		AbortIfNot(fcn_ptr2.attach(&test_funcs::func5), false);
		AbortIfNot(fcn_ptr3.attach(&test_funcs::func6), false);

		fcn_ptr2.bind("hello again!", 2);

		Signal::generic* gen_sig = &fcn_ptr2;
		gen_sig->v_raise();

		fcn_ptr2.raise("sup yo", 200);

		fcn_ptr1.raise();
		fcn_ptr3.raise();

		auto sig = dynamic_cast<
			Signal::signal_t<void,const std::string&,int>*
				>(fcn_ptr2.clone());

		sig->raise("I'm a fcn_ptr clone!", 42);
		auto sig2 = dynamic_cast<
			Signal::fcn_ptr<void,const std::string&,int>*
			>(sig);
		sig2->raise();
		delete sig;

		AbortIfNot(fcn_ptr1.detach(), false);
		AbortIfNot(fcn_ptr2.detach(), false);
		AbortIfNot(fcn_ptr3.detach(), false);
		AbortIf(fcn_ptr1.is_connected(), false);
		AbortIf(fcn_ptr2.is_connected(), false);
		AbortIf(fcn_ptr3.is_connected(), false);

		return true;
	}
};

class signal_test
{

public:

	bool run()
	{
		test_class obj1;
		test_class obj2;

		Signal::Signal<void> _sig_v;
		Signal::Signal<void,const std::string&, int> _sig_v_s_i;
		Signal::Signal<std::string> _sig_s;


		Signal::Signal<void>
			_sig_v2(&test_funcs::func1);

		Signal::Signal<void,const std::string&, int>
			_sig_v_s_i2(obj1, &test_class::func2);

		Signal::Signal<std::string>
			_sig_s2(obj1, &test_class::func3);


		AbortIfNot(_sig_v.attach(&test_funcs::func1),
			false);
		AbortIfNot(_sig_v_s_i.attach(obj1, &test_class::func2),
			false);
		AbortIfNot(_sig_s.attach(obj1, &test_class::func3),
			false);

		AbortIfNot(_sig_v.is_connected(), false);
		AbortIfNot(_sig_v_s_i.is_connected(), false);
		AbortIfNot(_sig_s.is_connected(), false);

		AbortIfNot(_sig_v2.is_connected(), false);
		AbortIfNot(_sig_v_s_i2.is_connected(), false);
		AbortIfNot(_sig_s2.is_connected(), false);

		auto sig_v      = std::move(_sig_v);
		auto sig_v_s_i  = std::move(_sig_v_s_i);
		auto sig_s      = std::move(_sig_s);

		auto sig_v2     = std::move(_sig_v2);
		auto sig_v_s_i2 = std::move(_sig_v_s_i2);
		auto sig_s2     = std::move(_sig_s2);

		sig_v.raise();
		sig_v_s_i.raise("hello", 3);
		sig_s.raise();

		sig_v2.raise();
		sig_v_s_i2.raise("hello again!", 4);
		sig_s2.raise();

		sig_v_s_i2.bind("hello yet again!", 5);

		Signal::generic* gen_sig = &sig_v_s_i2;
		gen_sig->v_raise();

		AbortIfNot(sig_v_s_i.attach(&test_class::func5),
			false);
		AbortIfNot(sig_s.attach(obj1, &test_class::func6),
			false);

		sig_v_s_i.raise("bye", 6);
		sig_s.raise();

		AbortIfNot(sig_v.attach(&test_funcs::func4),
			false);
		AbortIfNot(sig_v_s_i.attach(obj2, &test_class::func2),
			false);
		AbortIfNot(sig_s.attach(obj2, &test_class::func3),
			false);

		sig_v.raise();
		sig_v_s_i.raise("so long!", 7);
		sig_s.raise();

		sig_v_s_i.bind("binding for clone...", 999);
		auto sig(sig_v);
		sig.raise();
		auto sig2(sig_v_s_i);
		sig2.raise("I'm a clone!", 42);
		sig2.raise();

		// Verify Signal::clone() works
		auto sig3 = sig2.clone();
		sig3->v_raise();

		auto sig4 = dynamic_cast<
			Signal::Signal<  void,const std::string&, int>*>(sig3);
		sig4->raise("I'm sig4" , 4444);

		AbortIfNot(sig_v.detach(), false);
		AbortIfNot(sig_v_s_i.detach(), false);
		AbortIfNot(sig_s.detach(), false);
		AbortIfNot(sig_v2.detach(), false);
		AbortIfNot(sig_v_s_i2.detach(), false);
		AbortIfNot(sig_s2.detach(), false);

		AbortIf(sig_v.is_connected(), false);
		AbortIf(sig_v_s_i.is_connected(), false);
		AbortIf(sig_s.is_connected(), false);
		AbortIf(sig_v2.is_connected(), false);
		AbortIf(sig_v_s_i2.is_connected(), false);
		AbortIf(sig_s2.is_connected(), false);

		AbortIfNot(sig_v.attach(&test_funcs::func4),
			false);
		AbortIfNot(sig_v_s_i.attach(obj2, &test_class::func2),
			false);
		AbortIfNot(sig_s.attach(obj2, &test_class::func3),
			false);

		return true;
	}
};

class callable_test
{

public:

	bool run()
	{
		test_class obj;

		Signal::Callable<test_class> callable1;
		Signal::Callable<test_class> callable2(obj);

		AbortIf(callable1.is_connected(),
			false);
		AbortIfNot(callable2.is_connected(),
			false);

		AbortIfNot(callable1.attach(obj),
			false);
		AbortIfNot(callable1.is_connected(),
			false);

		callable1.raise("sup?");
		callable2.raise("not much.");

		AbortIfNot(callable1.detach(), false);
		AbortIfNot(callable2.detach(), false);

		AbortIf(callable1.is_connected(), false);
		AbortIf(callable2.is_connected(), false);

		return true;
	}
};

int main()
{
	mem_ptr_test test1;
	AbortIfNot(test1.run(), 1);

	std::cout << "\n";
	
	fcn_ptr_test test2;
	AbortIfNot(test2.run(), 1);

	std::cout << "\n";

	signal_test test3;
	AbortIfNot(test3.run(), 1);

	callable_test test4;
	AbortIfNot(test4.run(), 1);

	std::cout << "Test complete."
		<< std::endl;
	return 0;
}
