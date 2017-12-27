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
	  _protocol(nullptr),
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
}

/**
 * Initialize the engine
 *
 * @param[in] cmd_fd   The file descriptor through which to listen
 *                     for inputs from the GUI
 * @param[in] log_fd   The file descriptor used for logging
 * @param[in] protocol The communication protocol to use. See \ref
 *                     protocol.h for details
 *
 * @return True on success
 */
bool ChessEngine::init(int cmd_fd, int log_fd, protocol_t protocol)
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

	AbortIfNot(_protocol->init(cmd_fd),
		false);

	_state_machine = new StateMachine(_protocol->get_cmd_interface(),
		_logger);

	AbortIfNot(_state_machine->init(), false);

	/*
	 * Inform the state machine that _protocol may request state
	 * transitions:
	 */
	AbortIfNot(_state_machine->register_client(_protocol->get_name(),
		_protocol), false);

	_is_init = true;
	return true;
}

/**
 * Run the engine
 *
 * @param[in] algorithm The type of search algorithm to use
 *
 * @return True on success
 */
bool ChessEngine::run(algorithm_t algorithm)
{
	AbortIfNot(_is_init, false);

	/*
	 * 'search' allows us to easily experiment with different algorithms
	 * without changing the external behavior
	 */
	Search* search = nullptr;

	if (algorithm == pvs)
	{
		static MoveGen movegen(_tables);
		search = new PvSearch(movegen,*_state_machine,_logger, _tables);
	}
	else
	{
		Abort(false, "unsupported search algorithm.\n");
	}

	AbortIfNot(search->init(), false);

	/*
	 * Register the search algorithm with the state machine:
	 */
	AbortIfNot(_state_machine->register_client(
		search->get_name(), search), false);

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
			AbortIfNot(search->search(*_inputs), false);
			break;
		default:
			Output::to_stdout("unexpected state '%s'\n",
				state_name.c_str());

			Abort(false);
		}
	}

	delete(search);
	return true;
}
