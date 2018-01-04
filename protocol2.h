#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "output2.h"
#include "search2.h"

/**
 **********************************************************************
 *
 * @class Protocol
 *
 * A Protocol interacts with an external GUI via a pair of anonymous
 * pipes, one for chess engine's standard input and one for its
 * standard output. The pipes themselves are set up by the GUI so
 * that we only need to send and receive via standard output and
 * standard input, respectively. What exactly gets communicated is
 * protocol-specific, so this class represents a generic chess engine
 * communication protocol whose details are implemented by derived
 * classes
 *
 **********************************************************************
 */
class Protocol : public StateMachineClient, public OutputWriter
{

public:

	Protocol(const std::string& name, EngineInputs& _inputs,
			 Logger& logger);

	virtual ~Protocol();

	CommandInterface& get_cmd_interface();

	std::string get_name() const;

	virtual bool init(int fd, const Search* search) = 0;

	virtual bool postsearch(EngineOutputs& outputs) = 0;

	virtual bool send_periodics(const EngineOutputs& outputs)
		= 0;

	virtual bool sniff() = 0;


	/**
	 * Stuff user inputs here to be consumed by
	 * the search algorithm
	 */
	EngineInputs& inputs;

protected:

	/**
	 * The commanding interface which dispatches
	 * handlers for user commands
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
	const std::string
		_myname;
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
	 * Base class for engine-specific options, which we send
	 * to the GUI via the "option" command
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
			: name(_name), type(_type)
		{
		}

		/**
		 * Destructor
		 */
		virtual ~option_base() {}

		virtual bool update(const std::string& args)
			const = 0;

		/**
		 * The name of this option
		 */
		const std::string name;

		/**
		 * The option's UCI type
		 */
		const std::string type;
	};

	template <typename T>
	struct option : public option_base
	{
		typedef Signal::signal_t<void,T> signal_t;

		/**
		 * Constructor
		 *
		 * @param[in] name  The name of this option
		 * @param[in] type  One of the five types described in the UCI
		 *                  protocol
		 * @param[in] init  The option's default value
		 */
		option(const std::string& name, const std::string& type,
			   const T& init)
			: option_base(name, type), default_value(init),
			  _update_sig(nullptr)
		{
		}

		/**
		 * Destructor
		 */
		virtual ~option()
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
		 * The option's default value
		 */
		T default_value;

	protected:

		/**
		 * The callback that will update the engine after
		 * the "setoption" command is received
		 */
		signal_t* _update_sig;
	};

	struct Button : public option_base
	{
		typedef Signal::signal_t<void> signal_t;

		Button(const std::string& _name)
			: option_base(_name, "button"), _update_sig(nullptr)
		{
		}

		~Button()
		{
			if (_update_sig) delete _update_sig;
		}

		/**
		 * Assign a callback that will do the actual processing
		 * whenever this button is pushed
		 *
		 * @tparam C The class that implements the callback
		 *
		 * @param[in] obj  The object through which to call the
		 *                 callback
		 * @param[in] func The callback itself
		 *
		 * @return True on success
		 */
		template <typename C>
		bool assign_callback(C& obj, void(C::*func)())
		{
			AbortIf(_update_sig, false);

			_update_sig =
				new Signal::mem_ptr<void,C>(obj, func);
			AbortIfNot(_update_sig->is_connected(),
				false);

			return true;
		}

		/**
		 * Execute the callback. This should be called whenever
		 * the GUI issues the "setoption" command for this
		 * button
		 *
		 * @return True on success
		 */
		bool execute() const
		{
			AbortIfNot(_update_sig, false);
			AbortIfNot(_update_sig->is_connected(), false);

			_update_sig->raise();
			return true;
		}

	private:

		/**
		 * The callback that will be dispatched whenever
		 * the "setoption" command is received
		 */
		signal_t* _update_sig;
	};

	struct Check : public option<bool>
	{

		Check(const std::string& _name, bool init)
			: option(_name, "check", init)
		{
		}

		~Check() {}

		bool update(const std::string& args) const
		{
			AbortIfNot(_update_sig, false);
			AbortIfNot(_update_sig->is_connected(), false);

			bool value;
			if (!Util::from_string<bool>(args, value))
				return false;

			_update_sig->raise(value);
			return true;
		}
	};

	struct Combo : public option<std::string>
	{
		Combo(const std::string& _name, const std::string& init)
			: option(_name, "combo", init)
		{
		}

		virtual ~Combo() {}

		bool update(const std::string& args) const
		{
			AbortIfNot(_update_sig, false);
			AbortIfNot(_update_sig->is_connected(),
				false);

			for (auto iter = vars.begin(), end = vars.end();
				 iter != end; ++iter)
			{
				if (*iter == args)
				{
					_update_sig->raise(args);
					return true;
				}
			}

			return false;
		}

		Util::str_v vars;
	};

	struct Spin : public option<int>
	{
		Spin(const std::string& _name, int init, int _min, int _max)
			: option(_name, "spin", init),
			  min(_min), max(_max)
		{
		}

		~Spin() {}

		bool update(const std::string& args) const
		{
			AbortIfNot(_update_sig, false);
			AbortIfNot(_update_sig->is_connected(),
				false);

			int value;
			if (!Util::from_string<int>(args, value))
				return false;


			if (value > max || value < min)
				return false;

			_update_sig->raise(value);
			return true;
		}

		int min;
		int max;
	};

	struct String : public option<std::string>
	{
		String(const std::string& _name,
			   const std::string& init="<empty>")
			: option(_name, "string", init)
		{
		}

		~String() {}

		bool update(const std::string& args) const
		{
			AbortIfNot(_update_sig, false);
			AbortIfNot(_update_sig->is_connected(),
				false);

			_update_sig->raise(args);
			return true;
		}
	};

public:

	UCI(EngineInputs& inputs, Logger& logger);

	~UCI();

	bool debug(const std::string& _state);

	std::vector<option_base*>::iterator
		find_option(const std::string& name);

	bool go(const std::string& _args);

	bool init(int fd, const Search* search);

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

	bool _init_commands();

	bool _init_options();

	bool _init_outputs(const Search* search);

	/**
	 * The token used to look up the "bestmove" search
	 * output, which we send to the GUI at the end
	 * of every search
	 */
	int _bestmove_token;

	/**
	 * Options settable by the GUI
	 */
	std::vector<option_base*>
		_options;

	/**
	 * The token used to look up the "ponder" search
	 * output, which we send at the end of every
	 * search if pondering is enabled
	 */
	int _ponder_token;
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

	bool init(int fd, const Search* search);

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

	bool init(int fd, const Search* search);

	bool postsearch(EngineOutputs& outputs);

	bool send_periodics(const EngineOutputs& outputs);

	bool sniff();

private:

	bool    _is_init;
	Logger& _logger;
};

#endif
