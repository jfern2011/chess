#ifndef __CMD_H__
#define __CMD_H__

#include <iostream>
#include <map>
#include <string>

#include "log.h"
#include "ReadEventSink.h"

/**
 * @class CommandInterface
 *
 * Facilitates the installation and forwarding of user commands to their
 * respective handlers
 */
class CommandInterface
{
	typedef Signal::signal_t<bool,const std::string&> signal_t;

	struct cmd_info
	{
		cmd_info()
			: handler(nullptr), id(-1), name("")
		{
		}

		cmd_info(signal_t* _handler, int _id, const std::string& _name)
			: handler(_handler),
			  id(_id),
			  name(_name)
		{
		}

		cmd_info(cmd_info&& other)
		{
			*this =
				std::move(other);
		}

		~cmd_info()
		{
			if (handler) delete handler;
		}

		cmd_info& operator=(cmd_info&& rhs)
		{
			if (this != &rhs)
			{
				handler = rhs.handler;
				id = rhs.id;
				name = std::move(rhs.name);

				rhs.handler = nullptr;
				rhs.id = -1;
			}

			return *this;
		}

		cmd_info(const cmd_info& rhs)      = delete;
		cmd_info&
			operator=(const cmd_info& rhs) = delete;

		/*
		 * A handler to be dispatched whenever this command is issued
		 */
		signal_t* handler;

		/*
		 * A unique command ID
		 */
		int id;

		/*
		 * The name of this command
		 */
		std::string name;
	};

public:

	CommandInterface(Logger& logger);

	~CommandInterface();

	bool init(int fd);

    /**
	 * Register a new command whose handler is a non-const class
	 * member function
	 *
	 * @tparam C Class that implements the handler
	 *
	 * @param[in] name The name of this command
	 * @param[in] obj  The object on which to invoke the handler
	 * @param[in] func A pointer to the handler
	 *
	 * @return True on success
	 */
    template <typename C>
	bool install(const std::string& name, C& obj,
				 bool(C::*func)(const std::string&))
    {
    	signal_t* sig =
    		new Signal::mem_ptr<bool,C,const std::string&>(obj,func);

    	AbortIfNot(_install(name, sig), false);

        return true;
    }

    /**
	 * Registers a new command whose handler is a const class member
	 * function
	 *
	 * @tparam C Class that implements the handler
	 *
	 * @param[in] name The name of this command
	 * @param[in] obj  Object on which to invoke the command handler
	 * @param[in] func A pointer to the handler
	 *
	 * @return True on success
	 */
    template <typename C>
	bool install(const std::string& name, C& obj,
				 bool(C::*func)(const std::string&) const)
    {
    	signal_t* sig =
    		new Signal::mem_ptr<bool,C,const std::string&>(obj,func);

    	AbortIfNot(_install(name, sig), false);

        return true;
    }

    bool install(const std::string& name,
    			 bool(*func )( const std::string& ));

    bool is_init() const;

    bool is_installed(const std::string& name) const;

    bool poll();

    bool handle_command( const char* _input, size_t size );

private:

	bool _install(const std::string& _name, signal_t* sig);

	/**
	 * A record of registered commands
	 */
	std::map<std::string,cmd_info>
		_cmds;

	/**
	 * Initialized flag
	 */
	bool _is_init;

	/**
	 * Write diagnostic messages to this
	 */
	Logger& _logger;

	/**
	 * The name of this component
	 */
	const std::string _name;

	/**
	 * Listens for incoming commands
	 */
	ReadEventSink _res;
};

#endif
