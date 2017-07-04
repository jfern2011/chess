#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "cmd.h"
#include "DataTables.h"
#include "movegen.h"
#include "position.h"
#include "protocol.h"
#include "StateMachine.h"

#include <iostream>
#include <unistd.h>

class ChessEngine : private xBoardStateMachine
{

public:

	ChessEngine(const DataTables& tables)
		: _cmd(),
		  _movegen(tables),
		  _position(tables, true),
		  _quit(false),
		  _tables(tables),
		  _xboard(_position, *this)
	{
	}

	~ChessEngine()
	{
	}

	bool init()
	{
		AbortIfNot(init_commands(), false);

		std::cout << "Ready"  << std::endl;
		return true;
	}

	bool run()
	{
		while (!_quit)
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
		AbortIf(_cmd.install("new", _xboard, &xBoard::cmdNew) < 0,
				false);
		AbortIf(_cmd.install("divide", _xboard, &xBoard::divide2) < 0,
				false);
		AbortIf(_cmd.install("move", _xboard, &xBoard::move) < 0,
				false);
		AbortIf(_cmd.install("perft", _xboard, &xBoard::perft2) < 0,
				false);
		AbortIf(_cmd.install("quit", *this, &ChessEngine::quit) < 0,
				false);
		AbortIf(_cmd.install("setboard", _xboard, &xBoard::setboard) < 0,
				false);

		return true;
	}

	CommandInterface _cmd;
	MoveGen  _movegen;
	Position _position;
	bool     _quit;
	const DataTables& _tables;
	xBoard   _xboard;
};

#endif