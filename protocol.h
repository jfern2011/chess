#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include "clock.h"
#include "DataTables.h"
#include "position.h"
#include "search.h"
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

	xBoard(Node& node,
		   Position& position, xBoardStateMachine& state_machine)
		: _node(node),
		  _position(position),
		  _post(false),
		  _sd_limit(MAX_PLY ),
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

	/**
	 * @TODO Implement this
	 */
	bool divide3(const std::string& depth)
	{
		return true;
	}

	bool force(const std::string& args)
	{
		return
			_state_machine.updateState(xBoardStateMachine::FORCE);
	}

	bool go(const std::string& args)
	{
		bool success = 
			_state_machine.updateState(xBoardStateMachine::READY);

		int best_move;

		// Tell the node this is a new search rather than a
		// depth iteration:
		_node.restart_search();

		for (int depth = _min( 2, _sd_limit ); depth <= _sd_limit; depth++)
		{
			best_move = 0;

			int64 t_start = Clock::get_monotonic_time();

			_node.set_depth(depth);
			int score =
				_node.search(_position, best_move);

			int64 t_stop  = Clock::get_monotonic_time();

			if (abort_search())
				return true;

			/*
			 * Based on the search we just did, estimate the number of
			 * nodes to search such that 0.5 seconds will have
			 * elapsed. This will be the number of nodes to
			 * search before interrupting the search to check for user
			 * input:
			 */
			const double seconds =
				static_cast<double>(t_stop - t_start) / NS_PER_SEC;
			const int centiseconds = seconds * 100;

			if (seconds > 0)
			{
				const double nps = _node.get_node_count() /seconds;
				_node.set_input_check_delay(0.5*nps);
			}

			/*
			 * I don't expect xBoard to issue "go" when there are no moves
			 * available, but just in case...
			 */
			if (best_move == 0)
			{
				std::cout << "result ";

				if (score >= MATE_SCORE)
					std::cout << "1-0 {White Wins}" << std::endl;
				else if (score <= -MATE_SCORE)
					std::cout << "0-1 {Black Wins}" << std::endl;
				else
					std::cout << "1/2-1/2 {Draw}"   << std::endl;

				return true;
			}
			else
			{
				if (_post)
				{
					/*
					 * Saturate mate scores to a centipawn value of 100K,
					 * as required by xBoard
					 */
					if (score > 0)
						score = _min(score,  MATE_SCORE);
					else
						score = _max(score, -MATE_SCORE);

					const float scaled_score =
						static_cast<float>(score) / PAWN_VALUE;

					const int centipawns =
						static_cast<int>( scaled_score * 100 );

					std::cout
						<< _node.get_depth() << " "
						<< centipawns   << " ";
					if (_node.mate_found())
					{
						if (score > 0)
							std::cout << "+ ";
						else
							std::cout << "- ";
						std::cout << (_node.get_mate_plies() + 1)/2
							<< " ";
					}
					std::cout
						<< centiseconds << " "
						<< _node.get_node_count()
						<< " ";

					_node.get_pv( _position.toMove , _position.fullMove );
				}
			}
		}

		std::cout << "move "
			<< Util::printCoordinate(best_move)
			<< std::endl;

		return
			_position.makeMove(best_move);
	}

	bool nopost(const std::string& args)
	{
		return !(_post = false);
	}

	bool post(const std::string& args)
	{
		return ( _post = true );
	}

	bool usermove(const std::string& move)
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

	bool perft3(const std::string& depth)
	{
		MoveGen gen(tables);

		std::clock_t t = std::clock();

		int nodes= gen.perft3(_position, Util::str_to_int32(depth,10));

		t = std::clock() - t;

		std::cout << "nodes=" << nodes << " time="
			<< ((float)t)/CLOCKS_PER_SEC
			<< std::endl;

		return true;
	}

	bool perft4(const std::string& depth)
	{
		MoveGen gen(tables);

		std::cout << "Running performance test..."
			<< std::endl;

		std::clock_t t = std::clock();

		int nodes= gen.perft4(_position, Util::str_to_int32(depth,10));

		t = std::clock() - t;

		std::cout << "nodes=" << nodes << " time="
			<< ((float)t)/CLOCKS_PER_SEC
			<< std::endl;

		return true;
	}

	bool print(const std::string& args) const
	{
		char pieces[64];
		uint64 one = 1;

		for (int i = 0; i < 64; i++)
		{
			switch (_position.pieces[i])
			{
			case PAWN:
				if (_position.occupied[BLACK] & (one << i))
					pieces[i] = 'p';
				else
					pieces[i] = 'P';
				break;
			case KNIGHT:
				if (_position.occupied[BLACK] & (one << i))
					pieces[i] = 'n';
				else
					pieces[i] = 'N';
				break;
			case BISHOP:
				if (_position.occupied[BLACK] & (one << i))
					pieces[i] = 'b';
				else
					pieces[i] = 'B';
				break;
			case ROOK:
				if (_position.occupied[BLACK] & (one << i))
					pieces[i] = 'r';
				else
					pieces[i] = 'R';
				break;
			case QUEEN:
				if (_position.occupied[BLACK] & (one << i))
					pieces[i] = 'q';
				else
					pieces[i] = 'Q';
				break;
			case KING:
				if (_position.occupied[BLACK] & (one << i))
					pieces[i] = 'k';
				else
					pieces[i] = 'K';
				break;
			default:
				pieces[i] = ' ';
			}
		}

		Util::showPosition(pieces);
		return true;
	}

	bool sd(const std::string& _depth)
	{
		int depth = _min(Util::str_to_int32(_depth, 10), MAX_PLY);
		if (depth < 0)
			return false;
		else
			_sd_limit = depth;

		return true;
	}

	bool setboard(const std::string& fen)
	{
		if (!_position.reset(fen,true))
		{
			std::cout << "tellusererror Illegal position" << std::endl;
		}

		return true;
	}

	bool stat(const std::string& args)
	{
		_node.print_stats();
		return true;
	}

	bool undo(const std::string& args)
	{

	}

	bool white(const std::string& args)
	{

	}

private:

	bool abort_search() const
	{
		return _node.quit_requested() || _node.abort_requested();
	}

	Node&        _node;
	Position&    _position;
	bool         _post;
	int          _sd_limit;

	xBoardStateMachine&
			_state_machine;
};

#endif