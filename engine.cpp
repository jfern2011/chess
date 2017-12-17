#include "engine.h"

ChessEngine::ChessEngine(const DataTables& tables)
	: _is_init(false),
	  _logger(),
	  _position(tables, true),
	  _protocol(nullptr),
	  _state_machine(nullptr),
	  _tables(tables)
{
}

ChessEngine::~ChessEngine()
{
	if (_state_machine) delete _state_machine;
	if (_protocol) delete _protocol;
}

bool ChessEngine::init(int cmd_fd, int log_fd, protocol_t protocol)
{
	char msg[128];

	AbortIfNot(_logger.assign_fd(log_fd, true), false);

	switch (protocol)
	{
	case console_mode:
		_protocol = new Console(_logger);
		break;
	case uci_protocol:
		_protocol = new UCI(_logger);
		break;
	case xboard_protocol:
		_protocol = new xBoard(_logger);
		break;
	default:
		std::snprintf(msg, 128,
				"Invalid protocol ID: %d\n", protocol);
		Abort(false, msg);
	}

	AbortIfNot(_protocol->init(cmd_fd),
		false);

	_state_machine =
		new StateMachine(_protocol->get_cmd_interface(), _logger);

	AbortIfNot(_state_machine->init(), false);

	_is_init = true;
	return true;
}

bool ChessEngine::run()
{
	AbortIfNot(_is_init, false);
	return true;
}
