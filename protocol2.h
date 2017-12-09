#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "cmd.h"
#include "log.h"
#include "settings.h"

/**
 **********************************************************************
 *
 * @class Protocol
 *
 * A generic chess engine communication protocol, the details of which
 * are implemented by derived classes
 *
 **********************************************************************
 */
class Protocol
{

public:

	Protocol(Logger& logger);

	virtual ~Protocol();

	virtual bool init(int fd) = 0;

	virtual bool sniff() = 0;

	/*
	 * The engine settings which can be adjusted
	 * via this protocol
	 */
	EngineSettings settings;

protected:

	/**
	 * The commanding interface which dispatches
	 * command handlers
	 */
	CommandInterface _cmd;

	/**
	 * True if \ref init() was called
	 */
	bool _is_init;

	/**
	 * Writes to the chess engine log file
	 */
	Logger& _logger;
};

/**
 **********************************************************************
 *
 * @class UCI
 *
 * Implements the Universal Chess Interface (UCI) protocol
 *
 **********************************************************************
 */
class UCI : public Protocol
{
	/**
	 * Base class for engine-specific options
	 */
	struct option_base
	{
		/**
		 * Constructor
		 *
		 * @param [in] _name The name of this option
		 * @param [in] _type One of the five types described in the UCI
		 *                   protocol
		 */
		option_base(const std::string& _name, const std::string& _type)
			: name(_name),
			  type(_type)
		{
		}

		virtual std::string default_to_string() const = 0;
		virtual std::string min_to_string() const = 0;
		virtual std::string max_to_string() const = 0;
		virtual bool predefs_to_string(Util::str_v& strs )
			const = 0;

		/**
		 * The name of this option
		 */
		std::string name;

		/**
		 * The UCI option type
		 */
		std::string type;

		/**
		 * The number of inputs passed to the constructor. This is
		 * how we know which members are undefined; e.g. if only
		 * two arguments were passed to the constructor, then only
		 * \ref vars is defined
		 */
		size_t inputs;
	};

	/**
	 * Represents a single option
	 *
	 * @tparam T The data type used to store values for the option, not
	 *           to be confused with the five UCI types
	 */
	template <typename T>
	struct option : public option_base
	{
		/**
		 * Constructor (1)
		 *
		 * @param[in] name     The name of this option
		 * @param[in] type     One of the five types defined by the UCI
		 *                     protocol
		 * @param[in] _default The default value for this option
		 * @param[in] _min     The minimum value
		 * @param[in] _max     The maximum value
		 */
		option(const std::string& name,
			   const std::string& type,
			   const T& _default,
			   const T& _min,
			   const T& _max)

			: option_base(name, type),
			  default_value(_default),
			  min(_min),
			  max(_max),
			  vars()
		{
			// Identify which constructor was used
			inputs = 5;
		}

		/**
		 * Constructor (2)
		 *
		 * @param[in] name     The name of this option
		 * @param[in] type     One of the five types defined by the UCI
		 *                     protocol
		 * @param[in] _default The default value for this option
		 */
		option(const std::string& name,
			   const std::string& type,
			   const T& _default)

			: option_base(name, type),
			  default_value(_default),
			  min(),
			  max(),
			  vars()
		{
			// Identify which constructor was used
			inputs = 3;
		}

		/**
		 * Constructor (3)
		 *
		 * @param[in] name     The name of this option
		 * @param[in] type     One of the five types defined by the UCI
		 *                     communication protocol
		 */
		option(const std::string& name,
			   const std::string& type)

			: option_base(name, type),
			  default_value(),
			  min(),
			  max(),
			  vars()
		{
			// Identify which constructor was used
			inputs = 2;
		}

		/**
		 * Get the default value for this option as a string
		 *
		 * @return The default for this option
		 */
		std::string default_to_string() const
		{
			std::string out;
			AbortIfNot(Util::to_string(default_value, out),
				"");

			return out;
		}

		/**
		 * Get the minimum value for this option as a string
		 *
		 * @return The minimum for this option
		 */
		std::string min_to_string() const
		{
			std::string out;
			AbortIfNot(Util::to_string(min, out),
				"");
			
			return out;
		}

		/**
		 * Get the maximum value for this option as a string
		 *
		 * @return The maximum for this option
		 */
		std::string max_to_string() const
		{
			std::string out;
			AbortIfNot(Util::to_string(max, out),
				"");
			
			return out;
		}

		/**
		 * Get the set of predefined values for this option
		 *
		 * @param[in] strs The predefined values
		 *
		 * @return True on success
		 */
		bool predefs_to_string(Util::str_v& strs) const
		{
			strs.clear();

			for (auto iter = vars.begin(), end = vars.end();
				 iter != end; ++iter)
			{
				std::string str;
				AbortIfNot(Util::to_string(*iter, str),
					false);

				strs.push_back(str);
			}

			return true;
		}

		/**
		 * The default value for this option
		 */
		const T default_value;

		/**
		 * The minimum value for this option
		 */
		const T min;

		/**
		 * The maximum value for this option
		 */
		const T max;

		/**
		 * A set of pre-defined values
		 */
		std::vector<T> vars;
	};

public:

	UCI(Logger& logger);

	~UCI();

	bool debug(const std::string& _state);

	bool init(int fd);

	bool sniff();

	bool uci(const std::string&) const;

private:

	bool _init_options();

	bool _init_commands();

	/**
	 * The name of this class for logging purposes
	 */
	const std::string _name;

	/**
	 * Options settable by the GUI
	 */
	std::vector<option_base*>
		_options;
};

/**
 **********************************************************************
 *
 * @class xBoard
 *
 * Implements the xBoard/WinBoard communication protocol
 *
 **********************************************************************
 */
class xBoard : public Protocol
{

public:

	xBoard();

	~xBoard();

	bool init(int fd);

	bool sniff();

private:

	bool _init_commands();

};

#endif
