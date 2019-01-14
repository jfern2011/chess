# Signal: A lightweight C++ callback framework

Create customized "signals" that, when raised, are handled by user-
defined handlers. Signals can be tailored to run with class
methods, function pointers, and function objects in general, making
them well-suited for use in event-driven software.

To use this, simply include Signal.h and be sure to compile with
--std=c++11 (at least)

## Signal::mem_ptr

Use this class when your handler is a class member function. For
example:

	#include <string>
	#include <iostream>
     
	#include "Signal.h"
     
	// A class that implements a signal handler:
	class MyClass
	{
	public:
		bool my_handler(const std::string& s, int i) const
		{
			std::cout << "You passed in " << s << " and " << i
				<< std::endl;
			return true;
		}
	};
     
	int main()
	{
		MyClass mine;
        
		/*
		 * Construct the signal. Here, we indicate that our handler
		 * returns a bool, is implemented in MyClass, and takes a const
		 * string reference and int as arguments. Note that the first
		 * two template arguments are always required
		 */
		Signal::mem_ptr<bool,MyClass,const std::string&,int> sig(mine);
        
		/*
		 * Attach the signal handler:
		 */
		sig.attach(&MyClass::my_handler);
        
		/*
		 * Alternatively, you can construct/attach simultaneously:
		 */
		Signal::mem_ptr<bool,MyClass,const std::string&,int>
			sig2(mine, &MyClass::my_handler);
         
		/*
		 * Raise the signal!
		 */
		sig.raise("Hello", 12345);
         
		/*
		 * Bind input arguments to the signal:
		 */
		sig.bind("Goodbye", 54321);
         
		/*
		 * Now, you can raise the signal without explicitly passing
		 * inputs:
		 */
		sig.raise();
        
		return 0;
	}

## Signal::fcn_ptr

Use this class when your handler is a static function. For example:

	#include <string>
	#include <iostream>
     
	#include "Signal.h"
     
	// A static signal handler:
	void my_handler(int a, int b, int c, int d)
	{
		std::cout << " a = " << a << "\n";
		std::cout << " b = " << b << "\n";
		std::cout << " c = " << c << "\n";
		std::cout << " d = " << d << std::endl;
	}
     
	int main()
	{
		/*
		 * Default-construct the signal. Here, we indicate that our
		 * handler returns void and takes in four integers. Note that
		 * the first template argument (return type) is always
		 * required
		 */
		Signal::fcn_ptr<void,int,int,int,int> sig;
        
		/*
		 * Attach the signal handler:
		 */
		sig.attach(&my_handler);
        
		/*
		 * Alternatively, you can construct/attach simultaneously:
		 */
		Signal::fcn_ptr<void,int,int,int,int> sig2(my_handler);
        
		/*
		 * Raise the signal!
		 */
		sig.raise(1,2,3,4);
        
        /*
		 * Bind input arguments to the signal:
		 */
		sig.bind(4,3,2,1);
         
		/*
		 * Now you can raise the signal without explicitly passing
		 * inputs:
		 */
		sig.raise();
        
		return 0;
	}

## Signal::Signal

This class abstracts mem_ptr and fcn_ptr, allowing your handler to be
implemented as either a function pointer or a class method. For
example:

	#include <string>
	#include <iostream>
     
	#include "Signal.h"
     
	// A class that implements a signal handler:
	class MyClass
	{
	public:
		bool my_handler(const std::string& s, int i) const
		{
			std::cout << "You passed in " << s << " and " << i
				<< std::endl;
			return true;
		}
	};
     
	// A static signal handler:
	void my_handler(int a, int b, int c, int d)
	{
		std::cout << " a = " << a << "\n";
		std::cout << " b = " << b << "\n";
		std::cout << " c = " << c << "\n";
		std::cout << " d = " << d << std::endl;
	}
     
	// Another static signal handler:
	bool my_handler2(const std::string& s, int i)
	{
		std::cout << "You passed in " << s << " and " << i
			<< std::endl;
		return true;
	}
     
	int main()
	{
		/*
		 * 1. Default-construct a signal that returns a bool and takes
		 *    a const string reference and an int as arguments. Note
		 *    that we make no assumption as to whether the handler is
		 *    a static function or a class method:
		 */
		Signal::Signal<bool,const std::string&,int> sig1;
        
		/*
		 * 2. Construct a signal from a static function:
		 */
		Signal::Signal<void,int,int,int,int> sig2(&my_handler);
        
		/*
		 * 3. Construct a signal from a class method:
		 */
		MyClass mine;
		Signal::Signal<bool,const std::string&,int>
			sig3(mine, &MyClass::my_handler);
        
        /*
         * Attach a handler (that is a class method) to sig1:
         */
        sig1.attach(mine, &MyClass::my_handler);
        
        /*
         * Note that we could re-attach sig1 to a static function,
         * provided the template arguments don't change:
         */
        sig1.attach(&my_handler2);
        
        /*
         * Raise sig1:
         */
         sig1.raise("Hello", 12345);
         
        /*
         * Bind input arguments to sig2 and raise it:
         */
         sig2.bind(1,2,3,4);
         sig2.raise();
         
        /*
         * Bind input arguments to sig3 and raise it:
         */
        sig3.bind("Goodbye", 54321);
        sig3.raise();
        
		return 0;
	}

## Signal::signal_t

This is a base class for mem_ptr and fcn_ptr, if for some reason you
need to polymorphically reference the two

## Signal::generic

This is the base class of signal_t and requires no template arguments.
It only really exists because I thought it'd be convenient to be able
to have a container of signals, each with different template
parameters. For example, I might have a std::vector< generic >
containing all sorts of mem_ptrs and fcn_ptrs.

## Signal::Callable

Use this class when your handler is a callable object. For example:

	#include <string>
	#include <iostream>
     
	#include "Signal.h"
     
	struct MyStruct
	{
		void operator()(std::string str)
		{
			std::cout << str << std::endl;
		}
	};
     
	int main()
	{
		MyStruct mine1;
		MyStruct mine2;
        
		/*
		 * Default-construct and attach a handler:
		 */
		Signal::Callable<MyStruct> sig1;
		sig1.attach(mine1);
        
		/*
		 * Construct a signal with a given handler:
		 */
		Signal::Callable<MyStruct> sig2(mine2);
        
        /*
         * Raise the signals:
         */
		sig1.raise("May I interest you");
		sig2.raise("in a donut?");
        
		return 0;
	}

Honestly I don't see this being anywhere near as useful as the other
classes, but the namespace feels incomplete without it.

## Acknowledgements

A lot of the material here probably wouldn't exist (at least not for a
while) without help from Stack Overflow, particularly those who took
the time to talk to me about my template metaprogramming problems :)

## Contact

Feel free to email me with bug reports or suggestions:
jfernandez3@gatech.edu