#ifndef __VERBOSITY_H__
#define __VERBOSITY_H__

namespace Chess
{
	enum class Verbosity
	{
		quiet   = 0,
		terse   = 1,
		verbose = 2,
		debug   = 3
	};

	auto verbosity = Verbosity::terse;

	inline bool operator == (Verbosity v1, Verbosity v2)
	{
		return  static_cast<int>(v1) ==
				static_cast<int>(v2);
	}

	inline bool operator >= (Verbosity v1, Verbosity v2)
	{
		return  static_cast<int>(v1) >=
				static_cast<int>(v2);
	}

	inline bool operator <= (Verbosity v1, Verbosity v2)
	{
		return  static_cast<int>(v1) <=
				static_cast<int>(v2);
	}

	inline bool operator != (Verbosity v1, Verbosity v2)
	{
		return  static_cast<int>(v1) !=
				static_cast<int>(v2);
	}

	inline bool operator >  (Verbosity v1, Verbosity v2)
	{
		return  static_cast<int>(v1) >
				static_cast<int>(v2);
	}

	inline bool operator <  (Verbosity v1, Verbosity v2)
	{
		return  static_cast<int>(v1) <
				static_cast<int>(v2);
	}
}

#endif
