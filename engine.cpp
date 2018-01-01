#include "engine.h"
#include "output.h"

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
		_protocol = new Console(*_inputs, _logger);
		break;
	case uci_protocol:
		_protocol = new UCI(*_inputs, _logger);
		break;
	case xboard_protocol:
		_protocol = new xBoard(*_inputs, _logger);
		break;
	default:
		std::snprintf(msg, 128,
			"Invalid protocol ID: %d\n", protocol);
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
		_search = new PvSearch(
			_movegen,*_state_machine, _logger, _tables);
	}
	else
	{
		Abort(false, "unsupported search algorithm.\n");
	}

	AbortIfNot(_search->init(), false);

	AbortIfNot(_protocol->init(cmd_fd, _search),
		false);

	/*
	 * Allow the search algorithm to request state transitions:
	 */
	AbortIfNot( _state_machine->register_client(
		_search->get_name(), _search), false);

	/*
	 * Allow the protocol to request state transitions:
	 */
	AbortIfNot(_state_machine->register_client(
		_protocol->get_name(), _protocol), false);

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

	/*
	 * Poll for input every 100 ms when idle (not searching)
	 */
	const int sleep_time = 100000;

	auto update = [&]() { return _state_machine->get_current_state(); };

	while (update() != StateMachine::exiting)
	{
		const auto state = update();
		const std::string state_name = _state_machine->to_string(state);

		switch (state)
		{
		case StateMachine::idle:
			AbortIfNot(_protocol->sniff(), false);
			::usleep(sleep_time);
			break;
		case StateMachine::init_search:
			AbortIfNot(_search->search(*_inputs), false);
			break;
		case StateMachine::postsearch:
			AbortIfNot(_protocol->postsearch(
				_search->get_outputs()), false );
			break;
		default:
			Output::to_stdout("unexpected state: '%s'\n",
				state_name.c_str());

			Abort(false);
		}
	}

	return true;
}
