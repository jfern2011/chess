#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "eval.h"

class Node
{

public:

	/**
	 * Constructor
	 *
	 * @param[in] movegen A move generator object
	 * @param[in] save_pv Flag indicating whether or not to save the
	 *                    principal variation in searches
	 */
	Node(const MoveGen& movegen, bool save_pv=true)
		: _depth(1),
		  _evaluator(movegen), _movegen(movegen), _save_pv(save_pv)
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
	 * Print the principal variation obtained from the most recent
	 * search
	 */
	void get_pv() const
	{
		for (int ply = _depth; ply >= 0; --ply)
			std::cout << Util::printCoordinate(_pv[_depth][ply])
				<< " ";
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
			if (in_check)
				return (-sign) * SCORE_INF;
			else
				return 0;
		}

		int score = (-sign) * SCORE_INF;
		best_move = 0;

		for (register int i = 0; i < nMoves; i++)
		{
			bool raised_alpha = false;

			pos.makeMove(moves[i]);

			if (pos.toMove == flip(WHITE))
			{
				const int temp =
					-_search(pos, _depth-1, -SCORE_INF, SCORE_INF);

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
					 _search(pos, _depth-1, -SCORE_INF, SCORE_INF);

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
				_pv[_depth][_depth] = best_move;

				for (register int i = _depth-1; i >= 0; i--)
				{
					_pv[_depth][i] = _pv[_depth-1][i];
				}
			}
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
	const MoveGen& _movegen;
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
			return in_check ? (-SCORE_INF) : 0;
		}

		if (depth < 0)
			return sign * _evaluator.evaluate(pos);

		for (register int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);

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
