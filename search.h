#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <cstring>

#include "eval.h"

class Node
{

public:

	/**
	 * Constructor
	 *
	 * @param [in] movegen A move generator object
	 * @param [in] save_pv Flag indicating whether or not to save the
	 *                     principal variation in searches
	 */
	Node(const MoveGen& movegen, bool save_pv=true)
		: _depth(1),
		  _evaluator(movegen),
		  _mate_found(false),
		  _mate_plies(MAX_PLY),
		  _movegen(movegen),
		  _node_count(0),
		  	_save_pv( save_pv )
	{
	}

	/**
	 * Destructor
	 */
	~Node()
	{
	}

	/**
	 * Get the current search depth
	 *
	 * @return The current depth limit. Note that this does not include
	 *         quiescence search
	 */
	int get_depth() const
	{
		return _depth;
	}

	/**
	 * Determine if a checkmate was found during the most recent search
	 *
	 * @return True if a mate was found
	 */
	bool mate_found() const
	{
		return _mate_found;
	}

	/**
	 * The current number of plies to mate based on the most recent
	 * search
	 *
	 * @return Number of plies to mate, or MAX_PLY if no mate was
	 *         found
	 */
	int get_mate_plies() const
	{
		return _mate_plies;
	}

	/**
	 * Retrieve the most recent total number of nodes searched
	 *
	 * @return The node count
	 */
	int get_node_count() const
	{
		return _node_count;
	}

	/**
	 * Print the principal variation obtained from the most recent
	 * search
	 *
	 * @param[in] to_move   Side on move, e.g. WHITE
	 * @param[in] full_move Full move number
	 */
	void get_pv(int to_move, int full_move) const
	{
		const int end_ply =
					_mate_found ? (_depth-_mate_plies) : 0;

		for (int ply = _depth; ply > end_ply; )
		{
			int stop = 2;

			std::cout << full_move++ << ". ";
			if (to_move == BLACK && ply == _depth)
			{
				stop = 1;
					std::cout << "... ";
			}
			
			for (int i = 0; i < stop; i++)
			{
				std::cout << Util::printCoordinate(_pv[_depth][ply])
					<< " "; 
				if (--ply == end_ply)
					break;
			}
		}

		std::cout << std::endl;
	}

	/**
	 * Search for the best move from the given position
	 *
	 * @param[in]  pos       The current position
	 * @param[out] best_move The best move to play
	 *
	 * @return The score of the position
	 */
	int search(Position& pos, int& best_move)
	{
		uint32 moves[MAX_MOVES];

		const int sign = pos.toMove == WHITE ? 1 : -1;

		uint32* end;

		const bool in_check = pos.inCheck(pos.toMove);

		if (in_check)
			end =
			   _movegen.generateCheckEvasions(pos, pos.toMove, moves);
		else
			end = _movegen.generateLegalMoves(pos, pos.toMove, moves);

		const int nMoves = end - moves;

		if (nMoves == 0)
		{
			return
				in_check ? ((-sign) * MATE_SCORE) : 0;
		}

		_mate_found = false;

		int score = (-sign) * MATE_SCORE;
		best_move = 0;
		_node_count = 0;

		for (register int i = 0; i < nMoves; i++)
		{
			bool raised_alpha = false;

			pos.makeMove(moves[i]);
			_node_count++;

			if (pos.toMove == flip(WHITE))
			{
				const int temp =
					-_search(pos, _depth-1, -MATE_SCORE, MATE_SCORE);

				if (temp > score)
				{
					best_move = moves[i];
					score = temp;
					raised_alpha = true;
				}
			}
			else
			{
				const int temp =
					 _search(pos, _depth-1, -MATE_SCORE, MATE_SCORE);

				if (temp < score)
				{
					best_move = moves[i];
					score = temp;
					raised_alpha = true;
				}
			}

			pos.unMakeMove(moves[i]);

			/*
			 * Save the principal variation up to this node:
			 */
			if (_save_pv && (raised_alpha || i == 0))
			{
				_pv[_depth][_depth] = moves[i];

				for (register int i = _depth-1; i >= 0; i--)
				{
					_pv[_depth][i] = _pv[_depth-1][i];
				}
			}
		}

		if (best_move == 0)
			best_move = moves[0];

		/*
		 * Figure out the number of moves to checkmate:
		 */
		if (_abs(score) >= MATE_SCORE)
		{
			_mate_found = true;
			int ply = _depth;

			for (; _pv[_depth][ply] && ply > 0; ply--);

			_mate_plies = _depth - ply;
		}

		return score;
	}

	/**
	 * Set the depth limit for searches. The maximum depth saturates
	 * at MAX_PLY
	 *
	 * @param[in] depth Desired depth
	 *
	 * @return The new depth
	 */
	int set_depth(uint32 depth)
	{
		return (_depth = _min(MAX_PLY, depth));
	}

private:

	int            _depth;
	Evaluator      _evaluator;
	bool           _mate_found;
	int            _mate_plies;
	const MoveGen& _movegen;
	uint32         _node_count;
	int            _pv[MAX_PLY][MAX_PLY];
	bool           _save_pv;

	int _search(Position& pos, int depth, int alpha, int beta)
	{
		uint32 moves[MAX_MOVES];

		const int sign = pos.toMove == WHITE ? 1 : -1;

		uint32* end;

		const bool in_check = pos.inCheck(pos.toMove);

		if (in_check)
			end =
			   _movegen.generateCheckEvasions(pos, pos.toMove, moves);
		else
			end = _movegen.generateLegalMoves(pos, pos.toMove, moves);

		const int nMoves = end - moves;

		if (nMoves == 0)
		{
			//  Indicate this is the end of a variation
			//  with a null move:
			if (_save_pv)
				_pv[depth][depth] = 0;

			// Scale the mate score to favor checkmates
			// in fewer moves:
			return
				in_check ? ((-MATE_SCORE) * depth) : 0;
		}

		if (depth <= 0)
			return sign * _evaluator.evaluate(pos);

		for (register int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);
			_node_count++;

			const int score = -_search( pos, depth-1, -beta, -alpha );

			pos.unMakeMove(moves[i]);

			/*
			 * Save the principal variation up to this node:
			 */
			if (_save_pv && (score > alpha || i == 0))
			{
				_pv[depth][depth] = moves[i];

				for (register int i = depth-1; i >= 0; i--)
				{
					_pv[depth][i] = _pv[depth-1][i];
				}
			}

			alpha = _max(score,alpha);

			if (alpha >= beta)
				return beta;
		}

		return alpha;
	}
};

#endif
