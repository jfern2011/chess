#include "output.h"
#include "protocol2.h"

/**
 * Constructor
 */
Protocol::Protocol()
	: _cmd(), _is_init(false)
{
}

/**
 * Destructor
 */
Protocol::~Protocol()
{
}

/**
 * Construct a Universal Chess Interface
 */
UCI::UCI()
	: Protocol()
{
}

/**
 * Destructor
 */
UCI::~UCI()
{
	for (size_t i = 0; i < _options.size(); i++ )
		delete _options[i];
}

/**
 * Install the commands defined in this protocol
 *
 * @return True on success
 */
bool UCI::_init_commands()
{
	AbortIf(_is_init, false);

	AbortIfNot(_cmd.install<UCI>("uci", *this, &UCI::uci),
		false);

	return true;
}

/**
 * Define the parameters that can be set by the user via the GUI
 *
 * @return True on success
 */
bool UCI::_init_options()
{
	AbortIf(_is_init, false);

	/*
	 * Hash table size
	 */
	_options.push_back(new option<int>("Hash",  // Name
									   "spin",  // Type
									   16,      // Default (MB)
									   0,       // Min
									   65536)); // Max

	/*
	 * Pondering
	 */
	_options.push_back(new option<bool>("Ponder",
										"check",
										false));

	/*
	 * Make sure we have valid pointers:
	 */
	for (size_t i = 0; i < _options.size(); i++)
	{
		AbortIfNot(_options[i],
			false);
	}

	return true;
}

bool UCI::init(int fd)
{
	AbortIfNot(_cmd.init(fd),
		false);

	AbortIfNot(_init_commands(), false);
	AbortIfNot(_init_options(),  false);

	_is_init = true;
	return true;
}

bool UCI::sniff()
{
	AbortIfNot(_is_init, false);

	return _cmd.poll();
}

/**
 * Handles the "uci" command. Specifically, this replies to the GUI
 * with the chess engine ID info and a list of engine parameters
 * that can be set manually by the user. This list is complete when
 * we send the "uciok" message
 *
 * @return True on success
 */
bool UCI::uci(const std::string&) const
{
	AbortIfNot(_is_init, false);

	static const std::string name("Bender");

	AbortIfNot(Output::to_stdout("id name %s\n" , name.c_str()),
		false);
	AbortIfNot(Output::to_stdout("id author Jason Fernandez\n"),
		false);

	for (auto iter = _options.begin(), end = _options.end();
		 iter != end; ++iter)
	{
		const auto& ptr = *iter;

		switch (ptr->inputs)
		{
		case 5:
			AbortIfNot(Output::to_stdout(
				"option name %s type %s default %s min %s max %s",
				ptr->name.c_str(),
				ptr->type.c_str(),
				ptr->default_to_string().c_str(),
				ptr->min_to_string().c_str(),
				ptr->max_to_string().c_str()), false);
			break;
		case 3:
			AbortIfNot(Output::to_stdout(
				"option name %s type %s default %s",
				ptr->name.c_str(),
				ptr->type.c_str(),
				ptr->default_to_string().c_str()), false);
			break;
		case 2:
			AbortIfNot(Output::to_stdout(
				"option name %s type %s default %s",
				ptr->name.c_str(),
				ptr->type.c_str()), false);
			break;
		default:
			// We should never get here:
			Abort(false);
		}

		/*
		 * Send the predefined values (if any) to standard
		 * output:
		 */
		Util::str_v vars;
		AbortIfNot( ptr->predefs_to_string(vars),
			false);

		std::string predefs = "";
		for (size_t i = 0; i < vars.size(); i++)
		{
			predefs += " var " + vars[i];
		}

		AbortIfNot(Output::to_stdout("%s\n",
			predefs.c_str()), false);
	}

	/*
	 * Send the "uciok" footer:
	 */
	AbortIfNot(Output::to_stdout("uciok\n"),
		false);

	return true;
}
