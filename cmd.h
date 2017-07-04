#ifndef __CMD_H__
#define __CMD_H__

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <unistd.h>

#include "Signal.h"
#include "WriteEventSink.h"

/**
 **********************************************************************
 *
 * @class CommandInterface
 *
 * Facilitates the installation and forwarding of commands to their
 * respective handlers
 *
 **********************************************************************
 */
class CommandInterface
{

	typedef Signal::Signal<bool,const std::string&> read_sig_t;

	struct Command
	{
		Command(int _id, const std::string& _name,
				read_sig_t* _sig)
				: id(_id), name(_name), sig(_sig)
		{
		}

		Command(Command&& copy)
			: sig(NULL)
		{
			*this = std::move(copy);
		}

		~Command()
		{
			if (sig) delete sig;
		}

		Command& operator=(Command&& rhs)
		{
			if (this != &rhs)
			{
				if (sig) delete sig;

				id   = rhs.id;
				name = std::move( rhs.name );
				sig  = rhs.sig;

				rhs.sig = NULL;
			}

			return *this;
		}

		int         id;   // The command ID
		std::string name; // command name
		read_sig_t* sig;  // command handler
	};

	typedef std::vector<Command> cmd_v;

public:

	/**
	 * Constructor
	 */
	CommandInterface()
		: _cmd2id(), _commands(), _evt_sink(STDIN_FILENO)
	{
		if (!_evt_sink.attach_reader(*this,
				&CommandInterface::handle_command))
		{
			throw std::runtime_error(__PRETTY_FUNCTION__);
		}
	}

	/**
	 * Destructor
	 */
	~CommandInterface()
	{
	}

	/**
	 * Install a new command, where the command handler is a C-style
	 * function pointer
	 *
	 * @param[in] name The command name. Note that this is not case-
	 *                 sensitive
	 * @param[in] func A pointer to the command handler
	 *
	 * @return  A unique ID to associate with this command or, if \a
	 *          func and/or \a name is invalid, -1
	 */
	int install(const std::string& name, bool(*func)(const std::string&))
	{
		std::string _name =
			std::move(Util::to_lower(Util::trim(name)));
		AbortIf(_name.empty(), -1);

		if (isInstalled(_name))
			return _cmd2id[_name];

		int id = _commands.size();

		read_sig_t* sig = new read_sig_t(func);
		if (!sig->is_connected())
		{
			delete sig; return -1;
		}

		_commands.push_back(std::move(Command(id,_name,sig)));
			_cmd2id[_name] = id;

		return id;
	}

	/**
	 * Install a new command, where the command handler is a pointer
	 * to a class method
	 *
	 * @tparam C Class the implements the handler
	 *
	 * @param[in] name The command name. Note that this is not case-
	 *                 sensitive
	 * @param[in] obj  An instance of class C through which to
	 *                 invoke the handler
	 * @param[in] func A pointer to the command handler
	 *
	 * @return  A unique ID to associate with this command or, if \a
	 *          func and/or \a name is invalid, -1
	 */
	template <typename C>
	int install(const std::string& name,
				C& obj, bool(C::*func)(const std::string& args))
	{
		std::string _name =
			std::move(Util::to_lower(Util::trim(name)));
		AbortIf(_name.empty(), -1);

		if (isInstalled(_name))
			return _cmd2id[_name];

		int id = _commands.size();

		read_sig_t* sig = new read_sig_t(obj, func);
		if (!sig->is_connected())
		{
			delete sig; return -1;
		}

		_commands.push_back(std::move(Command(id,_name,sig)));
			_cmd2id[_name] = id;

		return id;
	}

	/**
	 * Install a new command, where the command handler is a pointer
	 * to a class method
	 *
	 * @tparam C Class the implements the handler
	 *
	 * @param[in] name The command name. Note that this is not case-
	 *                 sensitive
	 * @param[in] obj  An instance of class C through which to
	 *                 invoke the handler
	 * @param[in] func A pointer to the command handler
	 *
	 * @return  A unique ID to associate with this command or, if \a
	 *          func and/or \a name is invalid, -1
	 */
	template <typename C>
	int install(const std::string& name,
				C& obj, bool(C::*func)(const std::string& args) const)
	{
		std::string _name =
			std::move(Util::to_lower(Util::trim(name)));
		AbortIf(_name.empty(), -1);

		if (isInstalled(_name))
			return _cmd2id[_name];

		int id = _commands.size();

		read_sig_t* sig = new read_sig_t(obj, func);
		if (!sig->is_connected())
		{
			delete sig; return -1;
		}

		_commands.push_back(std::move(Command(id,_name,sig)));
			_cmd2id[_name] = id;

		return id;
	}

	/**
	 * Determine if a command has been installed by name. Note this is
	 * is case-insensitive
	 *
	 * @param [in] _cmd The name of the command
	 *
	 * @return True if the command is installed
	 */
	bool isInstalled(const std::string& _cmd)
	{
		std::string cmd = Util::trim(Util::to_lower( _cmd ));

		return _cmd2id.find(cmd)
					!= _cmd2id.end();
	}

	/**
	 * Determine if a command has been installed by ID. This is the ID
	 * returned by install()
	 *
	 * @param[in] id The command ID
	 *
	 * @return True if the command is installed
	 */
	bool isInstalled(int id)
	{
		for (size_t i = 0; i < _cmd2id.size(); i++)
		{
			if (_commands[i].id == id) return true;
		}

		return false;
	}

	/**
	 * Poll the standard input file descriptor for inputs, dispatching
	 * command handlers as needed
	 *
	 * @return True on success
	 */
	bool poll() //const
	{
		AbortIfNot(pollFd(), false);
		return true;
	}

private:

	/*
	 * Polls the standard input file descriptor for user commands
	 */
	bool pollFd()
	{
		WriteEventSink::err_code code =
					  _evt_sink.read(std::string("\n"));

		AbortIf(code != WriteEventSink::SUCCESS &&
			    code != WriteEventSink::NO_DATA, false);

		return true;
	}

	/*
	 * The callback routine dispatched by the event sink. Handles
	 * a single command
	 */
	bool handle_command(const char* _input, size_t size)
	{
		AbortIf(size == 0, false);

		std::string temp  = std::string(_input, size);
		std::string input =
			std::move(Util::trim(temp));

		Util::str_v tokens;
			Util::split( input,tokens );

		AbortIf(tokens.size() < 1, false);

		auto iter =
			_cmd2id.find(Util::to_lower(tokens[0]));

		if (iter == _cmd2id.end())
		{
			std::cout << "Error (unknown command): " << tokens[0]
				<< std::endl;
			return true;
		}

		const Command& cmd = _commands[iter->second];

		if (tokens.size() == 1)
			return cmd.sig->raise("");
		else
		{
			tokens.erase(tokens.begin());
			return
				cmd.sig->raise(Util::build_string(tokens,
								" "));
		}
	}

	std::map<std::string,int> _cmd2id;
	cmd_v _commands;
	WriteEventSink _evt_sink;
};

#endif
