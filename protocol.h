#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "DataTables.h"
#include "position.h"
#include "StateMachine.h"

/*
 * Databases used by the move generator
 */
extern DataTables tables;

/**
 **********************************************************************
 *
 * @class xBoard
 *
 * Contains command handlers that implement the xBoard communication
 * protocol
 *
 **********************************************************************
 */
class xBoard
{
public:

	xBoard(Position& position, xBoardStateMachine& state_machine)
		: _position(position),
		  _state_machine(state_machine)
	{
	}

	~xBoard()
	{
	}

	bool black(const std::string& args)
	{

	}

	bool divide(const std::string& depth)
	{
		MoveGen gen(tables);

		int nodes = gen.divide(_position,Util::str_to_int32(depth,10));

		std::cout << "\ntotal=" << nodes
			<< std::endl;

		return true;
	}

	bool divide2(const std::string& depth)
	{
		MoveGen gen(tables);

		int nodes= gen.divide2(_position,Util::str_to_int32(depth,10));

		std::cout << "\ntotal=" << nodes
			<< std::endl;

		return true;
	}

	bool force(const std::string& args)
	{
		return
			_state_machine.updateState(xBoardStateMachine::FORCE);
	}

	bool move(const std::string& move)
	{
		/*
		 * Note: parseCoordinate() retrieves the origin and
		 * destination squares, and promotion piece
		 */
		int move_bits = Util::parseCoordinate(move);
		if (move_bits == 0)
			return false;

		const int orig    = FROM(move_bits);
		const int dest    = TO(move_bits);
		const int promote = PROMOTE(move_bits);

		const piece_t moved    = _position.pieces[orig];
		const piece_t captured =
								 _position.pieces[dest];

		const int _move = pack(captured,
							   orig,
							   moved,
							   promote,
							   dest);

		if (MoveGen::isLegal(_position, _move))
			_position.makeMove(_move);
		else
			std::cout << "Illegal move: " << move
				<< std::endl;

		/*
		 * Don't abort; we've already messaged xBoard this
		 * move is illegal:
		 */
		return true;

	}

	bool cmdNew(const std::string& args)
	{
		bool success =
			_position.reset( "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
							true) &&
			_state_machine.updateState(xBoardStateMachine::READY);

		/*
		 * TODO (from xBoard protocol): Remove any search depth limit previously set by
		 * the sd command
		 */

		return success;
	}

	bool perft(const std::string& depth)
	{
		MoveGen gen(tables);

		std::clock_t t = std::clock();

		int nodes = gen.perft(_position, Util::str_to_int32(depth,10));

		t = std::clock() - t;

		std::cout << "nodes=" << nodes << " time="
			<< ((float)t)/CLOCKS_PER_SEC
			<< std::endl;

		return true;
	}

	bool perft2(const std::string& depth)
	{
		MoveGen gen(tables);

		std::clock_t t = std::clock();

		int nodes= gen.perft2(_position, Util::str_to_int32(depth,10));

		t = std::clock() - t;

		std::cout << "nodes=" << nodes << " time="
			<< ((float)t)/CLOCKS_PER_SEC
			<< std::endl;

		return true;
	}

	bool print(const std::string& args)
	{

	}

	bool setboard(const std::string& fen)
	{
		if (!_position.reset(fen,true))
		{
			std::cout << "tellusererror Illegal position" << std::endl;
		}

		return true;
	}

	bool undo(const std::string& args)
	{

	}

	bool white(const std::string& args)
	{

	}

private:
	Position&    _position;

	xBoardStateMachine&
			_state_machine;
};

#endif