#ifndef __CMD_H__
#define __CMD_H__

#include <iostream>
#include <map>

#include "WriteEventSink.h"

/**
 * @class CommandInterface
 *
 * Facilitates the installation and forwarding of commands to their
 * respective handlers
 */
class CommandInterface
{
	typedef Signal::generic signal_t;

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
		Signal::generic* handler;

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

	CommandInterface();

	~CommandInterface();

	/**
	 * Register a new command whose handler is a static function
	 *
	 * @tparam R The command handler's return type
	 * @tparam T Input argument types required by the handler
	 *
	 * @param[in] name The name of this command
	 * @param[in] func A function pointer to the command handler
	 *
	 * @return True on success
	 */
	template <typename R, typename... T>
	bool create(const std::string& name, R(*func)(T...))
    {
    	signal_t* sig =
    		new Signal::fcn_ptr<R,T...>(func);

    	AbortIfNot(_create(name, sig), false);

        return true;
    }

    /**
	 * Register a new command whose handler is a non-const class
	 * member function
	 *
	 * @tparam R The command handler's return type
	 * @tparam C Class that implements the handler
	 * @tparam T Input argument types required by the handler
	 *
	 * @param[in] name The name of this command
	 * @param[in] obj  The object on which to invoke the handler
	 * @param[in] func A pointer to the handler
	 *
	 * @return True on success
	 */
    template <typename R, typename C, typename... T>
	bool create(const std::string& name, C& obj, R(C::*func)(T...))
    {
    	signal_t* sig =
    		new Signal::mem_ptr<R,C,T...>(obj,func);

    	AbortIfNot(_create(name, sig), false);

        return true;
    }

    /**
	 * Registers a new command whose handler is a const class member
	 * function
	 *
	 * @tparam R The command handler's return type
	 * @tparam C Class that implements the handler
	 * @tparam T Input argument types required by the handler
	 *
	 * @param[in] name The name of this command
	 * @param[in] obj  object on which to invoke the command handler
	 * @param[in] func A pointer to the handler
	 *
	 * @return True on success
	 */
    template <typename R, typename C, typename... T>
	bool create(const std::string& name,
				C& obj, R(C::*func)(T...) const)
    {
    	signal_t* sig =
    		new Signal::mem_ptr<R,C,T...>(obj,func);

    	AbortIfNot(_create(name, sig), false);

        return true;
    }

    bool exists(const std::string& name) const;

private:

	bool _create(const std::string& _name, signal_t* sig);

	/**
	 * A record of registered commands
	 */
	std::map<std::string,cmd_info>
		_cmds;

	/**
	 * The file descriptor on which to listen for
	 * commands
	 */
	int _fd;
};

#endif
