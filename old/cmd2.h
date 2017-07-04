#ifndef __CMD_HANDLER__
#define __CMD_HANDLER__

#include <map>
#include <queue>
#include <vector>

#include "signal.h"
#include "util.h"
#include "WriteEventSink.h"

using namespace Signal;

struct cmd_data_t
{
	virtual ~cmd_data_t() {}
};

template <class R, class... T>
struct Command : public cmd_data_t
{
	Command()
		: id(-1), name(""), sig(NULL)
	{
	}

	int id;
	std::string name;
	signal_t<R,T...>* sig;
};

/**
 * A framework for forwarding commands to their assigned handlers
 */
class CommandRouter
{
	template <class... T>
	struct TypePack
	{
	};

	template <class T1, class... T2>
	struct TypePack<T1,T2...> : public TypePack<T2...>
	{
		T1 element;
	};

public:

	/**
	 * Default constructor
	 */
	CommandRouter()
		: _cmd2id(),
		  _free_slots(),
		  _id2sig()
	{
	}

	/**
	 * Deallocate all handlers
	 */
	~CommandRouter()
	{
		for (size_t i = 0; i < _id2sig.size(); i++)
			delete _id2sig[i];
	}

	/**
	 * Execute a command by name. Causes undefined behavior if the
	 * command is not registered
	 *
	 * @param[in] cmd  The name of the command to execute
	 * @param[in] args Inputs arguments to the handler
	 *
	 * @return The return value from the command handler
	 */
	template <typename R, typename... T>
	R forward(const std::string& cmd, T... args)
	{
		std::string _cmd = preprocess(cmd);
		int id = -1;

		if (_cmd2id.find(_cmd) == _cmd2id.end())
		{
			std::cout << "CommandRouter::forward() [Warning]: "
				<< "\"" << _cmd
				<< "\" is not a registered command: "
				<< std::endl;
		}
		else
			id = _cmd2id[_cmd];
/*
		signal_t<R,T...>* sig =
				dynamic_cast<signal_t<R,T...>*>(_id2sig[id]);
*/
		return _id2sig[id]->sig->raise(args...);
	}

	/**
	 * Execute a command by ID. Causes undefined behavior if the
	 * command is not registered
	 *
	 * @param[in] id   The ID of the command to execute
	 * @param[in] args Inputs arguments to the handler
	 *
	 * @return The return value from the command handler
	 */
	template <typename R, typename... T>
	R forward(int id, T... args)
	{
		if (_id2sig.size() <= id)
		{
			std::cout << "CommandRouter::forward() [Warning]: "
				<< "command ID = " << id
				<< " is out of range" << std::endl;
		}
/*
		signal_t<R,T...>* sig =
				dynamic_cast<signal_t<R,T...>*>(_id2sig[id]);
*/
		return _id2sig[id]->sig->raise(args...);
	}

	/**
	 * Retrieve the ID of a command. This is the ID returned when
	 * the command was installed
	 *
	 * @param[in] command The name of the command
	 *
	 * @return The command ID, or -1 if this command has not been
	 *         registered
	 */
	int getID(const std::string& command) const
	{
		std::string cmd = preprocess(command);

		if (_cmd2id.find(cmd) == _cmd2id.end())
			return -1;
		else
		{
			std::map<std::string,int>::const_iterator it =
				_cmd2id.find(cmd);
			return it->second;
		}
	}

	/**
	 * @brief
	 * Install a new command
	 *
	 * @details
	 * If successful, a unique ID is returned that can be used later
	 * to reference the command by. If the command is already
	 * installed, then the ID for this command is returned and no
	 * further action is taken. Therefore, if you wish to attach
	 * a different handler to this command, you must first uninstall
	 * it via uninstall()
	 *
	 * @param[in] cmd  The name of the command to install
	 * @param[in] func The handler for this command
	 *
	 * @return A unique integral identifier, or -1 if the install
	 *         failed
	 */
	template <typename R, typename... T>
	int install(const std::string& cmd, R(*func)(T...))
	{
		AbortIf(func == NULL, -1);

		const int id = pre_install(cmd);
		AbortIf(id < 0, -1);

		_id2sig[id]       = new Command<R,T...>();
		_id2sig[id]->id   = id;
		_id2sig[id]->name = preprocess(cmd);
		_id2sig[id]->sig  = new fcn_ptr<R,T...>(func);
/*
		fcn_ptr<R,T...>* sig =
			dynamic_cast<fcn_ptr<R,T...>*>(_id2sig[id]);
*/
		AbortIfNot(_id2sig[id]->sig->is_connected(), -1);
		return id;
	}

	/**
	 * @brief
	 * Install a new command
	 *
	 * @details
	 * If successful, a unique ID is returned that can be used later
	 * to reference the command by. If the command is already
	 * installed, then the ID for this command is returned and no
	 * further action is taken. Therefore, if you wish to attach
	 * a different handler to this command, you must first uninstall
	 * it via uninstall()
	 *
	 * @param[in] cmd  The name of the command to install
	 * @param[in] obj  The object through which to invoke the handler
	 *                 if the handler is a class method
	 * @param[in] func The handler for this command
	 *
	 * @return A unique integral identifier, or -1 if the install
	 *         failed
	 */
	template <typename R, typename C, typename... T>
	int install(const std::string& cmd, C& obj, R(C::*func)(T...))
	{
		AbortIf(func == NULL, -1);

		const int id = pre_install(cmd);
		AbortIf(id < 0, -1);

		_id2sig[id]       = new Command<R,T...>();
		_id2sig[id]->id   = id;
		_id2sig[id]->name = preprocess(cmd);
		_id2sig[id]->sig  = new mem_ptr<R,C,T...>(func);
/*
		_id2sig[id] = new mem_ptr<R,C,T...>(obj, func);

		mem_ptr<R,C,T...>* sig =
					dynamic_cast<mem_ptr<R,C,T...>*>(_id2sig[id]);
*/
		AbortIfNot(_id2sig[id]->sig->is_connected(), -1);
		return id;
	}

	/**
	 * Check if the given command has been installed
	 *
	 * @param[in] cmd The name of the command
	 *
	 * @return True if installed, false otherwise
	 */
	bool isInstalled(const std::string& cmd) const
	{
		return _cmd2id.find(preprocess(cmd)) != _cmd2id.end();
	}

	/**
	 * Check if the given command has been installed
	 *
	 * @param[in] id The command ID
	 *
	 * @return True if installed, false otherwise
	 */
	bool isInstalled(int id) const
	{
		return (id < _id2sig.size())
				&& (_id2sig[id] != NULL);
	}

	/**
	 * Uninstall a command by name
	 *
	 * @param[in] command The command
	 *
	 * @return True if the command was successfully uninstalled, false
	 *         otherwise
	 */
	bool uninstall(const std::string& command)
	{
		std::string cmd = preprocess(command);

		if (_cmd2id.find(cmd) == _cmd2id.end())
			return false;

		const int id = _cmd2id[cmd];
		_cmd2id.erase(cmd);

		delete _id2sig[id];
		_id2sig[id] = NULL;

		_free_slots.push(id);
		return true;
	}

	/**
	 * Uninstall a command by ID
	 *
	 * @param[in] id The command ID
	 *
	 * @return True if the command was successfully uninstalled, false
	 *         otherwise
	 */
	bool uninstall(int id)
	{
		if (!isInstalled(id))
			return false;

		for (std::map<std::string,int>::iterator it = _cmd2id.begin(),
			 end = _cmd2id.end(); it != end; ++it)
		{
			if (it->second == id)
				return uninstall(it->first);
		}

		return false;
	}

protected:

	/**
	 * Pre-install a new command. Allocates room for a new handler
	 * but does not attach it
	 *
	 * @param[in] cmd The name of the command to install
	 *
	 * @return  A unique integral identifier, or -1 if the install
	 *          failed
	 */
	int pre_install(const std::string& cmd)
	{
		std::string _cmd = preprocess(cmd);

		/*
		 * Sanity checks
		 */
		AbortIf(_cmd.empty(), -1);

		if (isInstalled(_cmd))
		{
			std::cout << "CommandRouter::install() [Warning]: "
				<< _cmd
				<< " is already installed"
				<< std::endl;
		}
		else
		{
			AbortIf(_cmd2id.find(_cmd) != _cmd2id.end(), -1);

			if (_free_slots.empty())
			{
				/*
			 	 * We need to grow the set of handlers. Push back
			 	 * an empty signal
			 	 */
				_cmd2id[_cmd] = _id2sig.size();
				_id2sig.push_back(NULL);
			}
			else
			{
				/*
				 *  Re-use a slot that belonged to a handler that
				 *  is now uninstalled
				 */
				_cmd2id[_cmd] = _free_slots.front();
				_free_slots.pop();
			}
		}

		return _cmd2id[_cmd];
	}

	/**
	 * Preprocess a command string before handing it off to forward()
	 * or install()
	 *
	 * @param[in] cmd The command
	 *
	 * @return The preprocessed string. This includes a conversion to
	 *         lower case followed by trimming
	 */
	std::string preprocess(const std::string& cmd) const
	{
		return Util::trim(Util::to_lower(cmd));
	}

	/**
	 * Mapping from command name to command ID
	 */
	std::map<std::string,int> _cmd2id;

	/**
	 * Set of indexes of _id2sig not currently
	 * in use
	 */
	std::queue<int> _free_slots;

	/**
	 * Mapping from command ID to handler
	 */
	std::vector<cmd_data_t*> _id2sig;
};


class CmdInterface : public CommandRouter, public WriteEventSink
{
public:
	CmdInterface(int fd)
		: CommandRouter(), WriteEventSink(fd),
		  _delim(0),
		  _is_init(false),
		  _use_delim(false)
	{
	}

	~CmdInterface()
	{
	}

	bool init()
	{
		_is_init = attach_reader(*this, &CmdInterface::reader);
		return _is_init;
	}

	bool run()
	{
		_use_delim ? read(_delim) : read();
		return true;
	}

private:

	bool reader(const char* data, size_t len)
	{
		AbortIfNot( _is_init, false);

		std::string _data(data, len);

		Util::str_v tokens;
		Util::split( _data, tokens );

		AbortIf(tokens.size() == 0, false);

		std::string cmd = preprocess(tokens[0]);
		std::string args;

		if (tokens.size() > 1)
		{
			size_t ind = _data.find(tokens[1]);
			std::string args =
				preprocess( _data.substr(ind, std::string::npos) );
		}

		if (isInstalled(cmd))
		{
			if (args.empty())
				return forward<bool>(cmd);
			else
				return forward<bool,const std::string&>(cmd, args);
		}

		return false;
	}

	/**
	 * Delimiter used during file descriptor
	 * reads
	 */
	char _delim;

	/**
	 * True if initialized
	 */
	bool _is_init;

	/**
	 * True if we're doing character-delimited
	 * reads
	 */
	bool _use_delim;
};

#endif
