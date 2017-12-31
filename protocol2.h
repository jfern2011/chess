#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "EngineOutputs.h"
#include "EngineInputs.h"
#include "StateMachine2.h"

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
class Protocol : public StateMachineClient
{

public:

	Protocol(const std::string& name, EngineInputs& _inputs,
			 Logger& logger);

	virtual ~Protocol();

	CommandInterface& get_cmd_interface();

	std::string get_name() const;

	virtual bool init(int fd)  = 0;

	virtual bool postsearch(EngineOutputs& outputs)
		= 0;

	virtual bool send_periodics(const EngineOutputs& outputs)
		= 0;

	virtual bool sniff() = 0;


	/**
	 * Stuff user inputs here, which will be consumed by the
	 * search algorithm
	 */
	EngineInputs& inputs;

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

	/**
	 * The name of this class for
	 * logging purposes
	 */
	const std::string _name;
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
		 * @param[in] _name  The name of this option
		 * @param[in] _type  One of the five types described in the UCI
		 *                   protocol
		 */
		option_base(const std::string& _name, const std::string& _type)
			: name(_name),
			  type(_type)
		{
		}

		virtual ~option_base() {}

		virtual std::string default_to_string() const = 0;
		virtual std::string min_to_string() const = 0;
		virtual std::string max_to_string() const = 0;
		virtual bool predefs_to_string (Util::str_v& strs)
			const = 0;
		virtual bool update(const std::string& value)
			const = 0;

		/**
		 * The name of this option
		 */
		std::string name;

		/**
		 * One of the UCI option types
		 */
		std::string type;

		/**
		 * The display type. This is purely used to respond
		 * to the "uci" command
		 */
		int display_type;
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
		typedef Signal::signal_t<void,T> signal_t;

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
			  _update_sig(nullptr),
			  default_value(_default),
			  min(_min),
			  max(_max),
			  vars()
		{
			display_type = 5;
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
			  _update_sig(nullptr),
			  default_value(_default),
			  min(),
			  max(),
			  vars()
		{
			display_type = 3;
		}

		/**
		 * Constructor (3)
		 *
		 * @param[in] name     The name of this option
		 * @param[in] type     One of the five types defined by the UCI
		 *                     protocol
		 */
		option(const std::string& name,
			   const std::string& type)

			: option_base(name, type),
			  _update_sig(nullptr),
			  default_value(),
			  min(),
			  max(),
			  vars()
		{
			display_type = 2;
		}

		/**
		 * Destructor
		 */
		~option()
		{
			if (_update_sig) delete _update_sig;
		}

		/**
		 * Assign the method that will actually update the engine's
		 * internals when this option is changed
		 *
		 * @tparam C The class that implements the updater function
		 *
		 * @param[in] obj  The object through which to call the
		 *                 updater
		 * @param[in] func The updater itself
		 *
		 * @return True on success
		 */
		template <typename C>
		bool assign_updater(C& obj, void(C::*func)(T))
		{
			AbortIf(_update_sig, false);

			_update_sig = new Signal::mem_ptr<void,C,T>(obj, func);
			AbortIfNot(_update_sig->is_connected(),
				false);

			/*
			 * Send the default value to the engine:
			 */
			_update_sig->raise(default_value);

			return true;
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
		 * @param[in] vals The predefined values
		 *
		 * @return True on success
		 */
		bool predefs_to_string(Util::str_v& vals) const
		{
			vals.clear();

			for (auto iter = vars.begin(), end = vars.end();
				 iter != end; ++iter)
			{
				std::string str;
				AbortIfNot(Util::to_string(*iter, str),
					false);

				vals.push_back(str);
			}

			return true;
		}

		/**
		 * Updates the engine with the current value of this
		 * option
		 *
		 * @param [in] value The value passed in by the user
		 *
		 * @return True on success
		 */
		bool update(const std::string& value) const
		{
			AbortIfNot(_update_sig, false);

			T val;
			if (!Util::from_string(value, val))
				return false;

			/*
			 * Attempt to match the input value against one of the
			 * predefined ones:
			 */
			if (!vars.empty())
			{
				for (size_t i = 0; i < vars.size(); i++)
				{
					if (vars[i] == val)
					{
						_update_sig->raise(val);
						return true;
					}
				}

				return false;
			}

			/*
			 * If this is a boolean option, then it makes no sense
			 * for us to perform bounds checking:
			 */
			if (Util::is_bool<T>::value)
			{
				_update_sig->raise(val);
				return true;
			}

			/*
			 * Otherwise, saturate the input value if needed:
			 */
			if (val > max)
				_update_sig->raise(max);
			else if (val < min)
				_update_sig->raise(min);
			else
				_update_sig->raise(val);

			return true;
		}

	private:

		/**
		 * The handler that will update the engine after
		 * the "setoption" command is sent
		 */
		signal_t* _update_sig;

	public:

		/**
		 * The default value for this option
		 */
		const T default_value;

		/**
		 * The minimum value for the option
		 */
		const T min;

		/**
		 * The maximum value for the option
		 */
		const T max;

		/**
		 * A set of pre-defined values
		 */
		std::vector<T> vars;
	};

public:

	UCI(EngineInputs& inputs, Logger& logger);

	~UCI();

	bool debug(const std::string& _state);

	std::vector<option_base*>::iterator
		find_option(const std::string& name);

	bool go(const std::string& _args);

	bool init(int fd);

	bool isready(const std::string&) const;

	bool position(const std::string& _args)  const;

	bool postsearch(EngineOutputs& outputs);

	bool quit(const std::string&);

	bool register_engine(const std::string&) const;

	bool send_periodics(const EngineOutputs& outputs);

	bool setoption(const std::string& _args);

	bool sniff();

	bool uci(const std::string&) const;

	bool ucinewgame(const std::string&);

private:

	bool _init_options();

	bool _init_commands();

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

	xBoard(EngineInputs& inputs, Logger& logger);

	~xBoard();

	bool init(int fd);

	bool postsearch(EngineOutputs& outputs);

	bool send_periodics(const EngineOutputs& outputs);

	bool sniff();

private:

	bool _init_commands();

	bool    _is_init;
	Logger& _logger;
};

/**
 **********************************************************************
 *
 * @class Console
 *
 * Used for interfacing via a terminal
 *
 **********************************************************************
 */
class Console : public Protocol
{

public:

	Console(EngineInputs& inputs, Logger& logger);

	~Console();

	bool init(int fd);

	bool postsearch(EngineOutputs& outputs);

	bool send_periodics(const EngineOutputs& outputs);

	bool sniff();

private:

	bool    _is_init;
	Logger& _logger;
};

#endif
