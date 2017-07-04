#ifndef __ENGINE__
#define __ENGINE__

#include <iostream>

#include "DataTables.h"
#include "movegen.h"

class Engine
{

public:

	/**
	 **********************************************************************
	 *
	 * Default constructor
	 *
	 **********************************************************************
	 */
	Engine()
		: _data_tables(),
		  _mv_generator(_data_tables),
		  _position(_data_tables),
		  _state(IDLE)
	{
	}

	~Engine()
	{
	}

	bool move(int32 move)
	{
		// validate move
		_position.makeMove(move);
		return true;
	}

	int perft(int depth, int to_move)
	{
		int nMoves = 0, nodes = 0;
		uint32 moves[MAX_MOVES];
		
		uint32* end =
			_mv_generator.generateCaptures(_position,
										   to_move,
										   moves); // TODO: If captured king, return?

		end =
			_mv_generator.generateNonCaptures(_position,
				                              to_move,
				                              end);
		nMoves = end - moves;

		for (int i = 0; i < nMoves; i++)
		{
			_position.makeMove(moves[i]);
			if (!_position.inCheck(to_move))
			{
				if (depth <= 0)
					nodes += 1;
				else
					nodes += perft(depth-1, flip(to_move));
			}
			_position.unMakeMove(moves[i]);
		}

		return nodes;
	}

	void divide(int depth)
	{
		const int to_move = _position.getTurn();
		int nMoves;
		uint32 moves[MAX_MOVES];
		
		uint32* end =
			_mv_generator.generateCaptures(_position,
										   to_move,
										   moves);

		end =
			_mv_generator.generateNonCaptures(_position,
				                              to_move,
				                              end);
		nMoves = end - moves;
		int nlegal = nMoves;

		int* nodes = new int[nMoves];
		std::memset(nodes, 0, nMoves*sizeof(int));

		for (int i = 0; i < nMoves; i++)
		{
			Position pos(_position); // test

			_position.makeMove(moves[i]);

			if (!_position.inCheck(to_move))
			{
				if (depth <= 0)
					nodes[i]++;
				else
					nodes[i] = perft(depth-1, flip(to_move));
			}
			else
				nlegal--;

			_position.unMakeMove(moves[i]);
/*
			if (!(pos == _position)) // test
			{
				std::cout << "error: " << SQUARE_STR[FROM(moves[i])]
					<< SQUARE_STR[TO(moves[i])]
					<< ", depth = "<< depth << std::endl;
				std::cout << "Original:\n";
				pos.debugPrint();
				std::cout << "New:\n";
				_position.debugPrint();
				std::cin.get();
			}
*/
		}

		uint32 count = 0;
		for (int i = 0; i < nMoves; i++)
		{
			std::cout << SQUARE_STR[FROM(moves[i])]
		    	      << SQUARE_STR[TO(moves[i])]
		        	  << ": " << nodes[i] << std::endl;
			count += nodes[i];
		}

		std::cout << "Nodes = " << count << "\n"
			 << "Moves = " << nlegal << std::endl;

		delete[] nodes;
	}

	bool runPerft(int depth)
	{
		const int to_move = _position.getTurn();
		const int nodes = perft(depth, to_move);

		std::cout << "Nodes = " << nodes
			<< std::endl;

		return true;
	}

	bool undo(int32 move)
	{
		// validate move
		_position.unMakeMove(move);
		return true;
	}

	bool setBoard(const std::string& fen)
	{
		return _position.reset(fen);
	}

	bool debugMakeMove(int32 move)
	{
		Position temp(_position);
		_position.makeMove(move);
		_position.unMakeMove(move);

		return temp == _position;
	}

//=====================================================================
// Commanding	
//=====================================================================

	bool force(const Util::str_v& args)
	{
		_state = FORCE;

		std::cout << "Force mode enabled."
			<< std::endl;
		return true;
	}

	typedef enum
	{
		IDLE,
		FORCE,
		N_STATES
	} state_t;

private:

	DataTables _data_tables;
	MoveGen    _mv_generator;
	Position   _position;
	state_t    _state;
};

#endif