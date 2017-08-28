#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "cmd.h"
#include "DataTables.h"
#include "movegen.h"
#include "position.h"
#include "protocol.h"
#include "search.h"
#include "StateMachine.h"

#include <iostream>
#include <unistd.h>

class ChessEngine : private xBoardStateMachine
{

public:

	ChessEngine(const DataTables& tables)
		: _cmd(),
		  _movegen(tables),
		  _node(_movegen),
		  _position(tables, true),
		  _quit(false),
		  _tables(tables),
		  _xboard(_node, _position, *this)
	{
	}

	~ChessEngine()
	{
	}

	bool init()
	{
		AbortIfNot(init_commands(), false);
		AbortIfNot(_node.init(), false);

		std::cout << "Ready"  << std::endl;
		return true;
	}

	bool run()
	{
		while (!_quit && !_node.quit_requested())
		{
			::usleep(100000); // 100 ms
			_cmd.poll();
		}

		return true;
	}

	bool quit(const std::string& args)
	{
		_quit = true;
		return _quit;
	}

private:

	bool init_commands()
	{
		AbortIf(_cmd.install("divide", _xboard, &xBoard::divide2) < 0,
				false);
		AbortIf(_cmd.install("go", _xboard, &xBoard::go) < 0,
				false);
		AbortIf(_cmd.install("usermove", _xboard, &xBoard::usermove) < 0,
				false);
		AbortIf(_cmd.install("new", _xboard, &xBoard::cmdNew) < 0,
				false);
		AbortIf(_cmd.install("nopost", _xboard, &xBoard::nopost) < 0,
				false);
		AbortIf(_cmd.install("perft", _xboard, &xBoard::perft2) < 0,
				false);
		AbortIf(_cmd.install("post", _xboard, &xBoard::post) < 0,
				false);
		AbortIf(_cmd.install("print", _xboard, &xBoard::print) < 0,
				false);
		AbortIf(_cmd.install("quit", *this, &ChessEngine::quit) < 0,
				false);
		AbortIf(_cmd.install("sd", _xboard, &xBoard::sd) < 0,
				false);
		AbortIf(_cmd.install("setboard", _xboard, &xBoard::setboard) < 0,
				false);
		AbortIf(_cmd.install("stat", _xboard, &xBoard::stat) < 0,
				false);

		return true;
	}

	CommandInterface _cmd;
	MoveGen  _movegen;

	// Note: _node requires a constructed
	//       _movegen
	Node     _node; 
	Position _position;
	bool     _quit;
	const DataTables& _tables;
	xBoard   _xboard;
};

#endif