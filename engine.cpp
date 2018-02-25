#include "engine.h"

/**
 * Constructor
 *
 * @param[in] tables The set of global pre-computed databases
 */
ChessEngine::ChessEngine(const DataTables& tables)
	: _inputs(nullptr),
	  _is_init(false),
	  _logger(),
	  _movegen(tables),
	  _name("ChessEngine"),
	  _protocol(nullptr),
	  _search(nullptr),
	  _state_machine(nullptr),
	  _tables(tables)
{
}

/**
 * Destructor
 */
ChessEngine::~ChessEngine()
{
	if (_state_machine) delete _state_machine;
	if (_inputs) delete _inputs;
	if (_protocol) delete _protocol;
	if (_search) delete _search;
}

/**
 * Initialize the engine
 *
 * @param[in] algorithm The type of search algorithm to use
 * @param[in] cmd_fd    The file descriptor through which to listen
 *                      for inputs from the GUI
 * @param[in] log_fd    The file descriptor used for logging
 * @param[in] protocol  The communication protocol to use. See \ref
 *                      protocol.h for details
 *
 * @return True on success
 */
bool ChessEngine::init(algorithm_t algorithm, int cmd_fd, int log_fd,
	protocol_t protocol)
{
	char msg[128];

	AbortIfNot(_logger.assign_fd(log_fd, true), false);

	_inputs = new EngineInputs(_tables, _logger);
	AbortIfNot(_inputs->init(Position(_tables, true )),
		false);

	switch (protocol)
	{
	case console_mode:
		_protocol = new Console(_tables, *_inputs, _logger);
		break;
	case uci_protocol:
		_protocol = new UCI(_tables, *_inputs, _logger);
		break;
	case xboard_protocol:
		_protocol = new xBoard(_tables, *_inputs, _logger);
		break;
	default:
		std::snprintf(msg, 128,
			"Invalid protocol ID: %d", protocol);
		Abort(false, msg);
	}

	_state_machine = new StateMachine(_protocol->get_cmd_interface(),
		_logger);

	AbortIfNot(_state_machine->init(), false);

	/*
	 * Create the search algorithm. This will also create the outputs
	 * that the protocol forwards to the GUI
	 */
	if (algorithm == pvs)
	{
		_search = new PvSearch(_movegen,
							   *_state_machine,
							   _logger,
							   _protocol,
							   _tables);
	}
	else
	{
		std::snprintf(msg,
			128, "unsupported search algorithm [%d]\n",
			static_cast<int>(algorithm));

		Abort(false, msg);
	}

	AbortIfNot(_search->init(), false);

	AbortIfNot(_protocol->init(cmd_fd, _search),
		false);

	/*
	 * Build the state machine. This creates the list of
	 * tasks to run in each state:
	 */
	AbortIfNot(_build_state_machine(),
		false);

	_is_init = true;
	return true;
}

/**
 * Run the engine
 *
 * @return True on success
 */
bool ChessEngine::run()
{
	AbortIfNot(_is_init, false);

	auto update = [&]()
	{
		return _state_machine->get_current_state();
	};

	while (update() != StateMachine::exiting)
	{
		AbortIfNot( _state_machine->run(), false );
	}

	return true;
}

/**
 * Build the state machine. This creates task lists that run depending
 * on the current state
 *
 * @return True on success
 */
bool ChessEngine::_build_state_machine()
{
	AbortIfNot(_protocol, false);
	AbortIfNot(_search,   false);

	/*
	 * Create the task(s) to perform while we're in StateMachine::idle
	 */
	{
		auto task = new Task<bool>("sniff");
		AbortIfNot(task->attach( *_protocol, &Protocol::sniff ),
			false);

		AbortIfNot( _state_machine->add_task(StateMachine::idle,
			task), false);
	}

	{
		auto task = new Task<int,useconds_t>("usleep");
		AbortIfNot(task->attach(&::usleep),
			false);
		/*
		 * When idle, poll for input every 100 ms to reduce wasted CPU
		 * time:
		 */
		const int sleep_time = 100000;
		task->bind(sleep_time);

		AbortIfNot(_state_machine->add_task(StateMachine::idle, task),
			false);
	}

	/*
	 * Create the task(s) to perform when in StateMachine::init_search
	 */
	{
		auto task = new Task<bool,const EngineInputs*>("search");
		AbortIfNot(task->attach(*_search, &Search::search),
			false);

		task->bind(_inputs);

		AbortIfNot(_state_machine->add_task(StateMachine::init_search,
			task), false);
	}

	/*
	 * Create the task(s) to perform when in StateMachine::post_search
	 */
	{
		auto task = new Task<bool,EngineOutputs*>("postsearch");
		AbortIfNot( task->attach( *_protocol, &Protocol::postsearch ),
			false);

		task->bind(_search->get_outputs());

		AbortIfNot(_state_machine->add_task(StateMachine::post_search,
			task), false);
	}

	/*
	 * Allow the protocol to request state transitions:
	 */
	AbortIfNot(_state_machine->register_client(_protocol),
		false);

	/*
	 * Allow the search algorithm to request state transitions:
	 */
	AbortIfNot( _state_machine->register_client(_search ),
		false);

	return true;
}
