#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <cstring>

#include "eval.h"

class Node
{
	friend class SEE_UT;

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
		  _qnode_count(0),
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
		for (int ply = 0; _pv[0][ply]; )
		{
			int stop = 2;

			std::cout << full_move++ << ". ";
			if (to_move == BLACK && ply == 0)
			{
				stop = 1;
					std::cout << " ... ";
			}
			
			for (int i = 0; i < stop; i++)
			{
				if (!_pv[0][ply])
					break;
				std::cout << Util::printCoordinate(_pv[0][ply])
					<< " ";
				ply++;
			}
		}

		std::cout
			<< std::endl;
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

		const int init_score = MATE_SCORE * 2 * MAX_PLY;

		const int init_alpha = -init_score;
		const int init_beta  =  init_score;

		_mate_found = false;

		int score = (-sign) * init_score;
		best_move = 0;
		_node_count = _qnode_count = 0;

		/*
		 * Clear the principal variation. Note that PV read-out ends
		 * when we hit the first null move
		 */
		for (register int i = 0; i < MAX_PLY; i++)
			_pv[0][i] = 0;

		for (register int i = 0; i < nMoves ; i++)
		{
			bool raised_alpha = false;

			pos.makeMove(moves[i]);
			_node_count++;

			if (pos.toMove == flip(WHITE))
			{
				const int temp = -_search(pos,1,init_alpha,init_beta);

				if (temp > score)
				{
					best_move= moves[i];
					score = temp;
					raised_alpha = true;
				}
			}
			else
			{
				const int temp =  _search(pos,1,init_alpha,init_beta);

				if (temp < score)
				{
					best_move= moves[i];
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
				_pv[0][0] = moves[i];

				for ( register int j = 1; j < _depth; j++ )
				{
					_pv[0][j] = _pv[1][j];
				}
			}
		}

		/*
		 * Figure out the number of moves to checkmate:
		 */
		if (_abs(score) >= MATE_SCORE)
		{
			_mate_found = true;
			int ply = 0;

			for (; _pv[0][ply] && ply < MAX_PLY; ply++);

			_mate_plies = ply-1;
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
	uint32         _qnode_count;
	bool           _save_pv;

	/**
	 * Bubble sort algorithm. This is used for move ordering (at least
	 * for now)
	 *
	 * @param[in] items A list of items
	 * @param[in] numel The number of elements to sort
	 *
	 * @return The number of passes performed as a result of having to
	 *         swap items
	 */
	inline int bubbleSort(uint32* items, int numel)
	{
		bool swapped = true;
		int passes = 0;

		while (swapped)
		{
			swapped = false;

			for (register int i = 1; i < numel; i++)
			{
				if (!compare(items[i-1],items[i]))
				{
					swapUint32(items[i-1], items[i]);
					swapped = true;
				}
			}
			passes++;

			// We've sorted the last element:
			numel--;
		}

		return passes;
	}

	/**
	 * Compare two captures. This is used by quiesce() to sort its list
	 * of captures
	 *
	 * Captures are compared using the MVV/LVA approach, e.g. PxQ is
	 * ordered before PxR
	 *
	 * @param[in] a The first value
	 * @param[in] b The value to compare the first against
	 *
	 * @return True if \a b is less than \a a; returns false otherwise
	 */
	inline static bool compare(uint32 a, uint32 b)
	{
		int gain_a, gain_b;

		/*
		 * If this is a non-capture promotion, assign it a neutral
		 * value so that it gets searched prior to losing captures:
		 */
		if (PROMOTE(a) && !CAPTURED(a))
			gain_a = 0;
		else
			gain_a = Evaluator::piece_value[CAPTURED(a)] -
					 Evaluator::piece_value[MOVED(a)];

		if (PROMOTE(b) && !CAPTURED(b))
			gain_b = 0;
		else
			gain_b = Evaluator::piece_value[CAPTURED(b)] -
					 Evaluator::piece_value[MOVED(b)];

		return
			gain_b <= gain_a;
	}

	/**
	 * Quiescence search. This is called from _search() once the search
	 * depth is exhausted
	 */
	int quiesce(Position& pos, int depth, int alpha, int beta)
	{
		uint32 moves[MAX_MOVES];
		uint32* end;

		const int sign = pos.toMove == WHITE ? 1 : -1;

		const bool in_check = pos.inCheck(pos.toMove);

		if (in_check)
		{
			end =
			   _movegen.generateCheckEvasions(pos, pos.toMove, moves);

			if (moves == end)
			{
				//  Indicate this is the end of a variation
				//  with a null move:
				if (_save_pv)
					_pv[depth][depth] = 0;

				// Scale the mate score to favor checkmates
				// in fewer moves:
				return
					(-MATE_SCORE) * (MAX_PLY-depth);
			}
		}
		else
			  end = _movegen.generateCaptures(pos, pos.toMove, moves);

		const int nMoves = end - moves;

		/*
		 * Compute an initial score for this position:
		 */
		const int score =
			sign * _evaluator.evaluate(pos);

		/*
		 * Check if we can "fail-high." Not sure if this is correct for
		 * zugzwang positions...
		 */
		if (score >= beta)
			return beta;

		bool raised_alpha = false;

		if (alpha <= score)
		{
			/*
			 * Set the flag indicating we want to save the PV that lead
			 * to this score:
			 */
			raised_alpha  =  true;
			alpha = score;
		}

		/*
		 * Return the heuristic value of this position if
		 * no captures are left:
		 */
		if (nMoves == 0 || MAX_PLY <= depth)
		{
			if (_save_pv)
				_pv[depth][depth] = 0;
			return score;
		}

		/*
		 * Sort the capture list. Captures are generated starting with
		 * pawns, knights/bishops, rooks, queens, and finally kings.
		 * The idea in starting captures with the least valuable
		 * pieces is to minimize the number of swaps performed by the 
		 * bubble sort algorithm
		 */
		if (!in_check)
			bubbleSort(moves, nMoves);

		int best_index = -1;

		for (register int i = 0; i < nMoves; i++)
		{
			if (!in_check)
			{
				/*
				 * Perform a see() on captures which might be losing,
				 * e.g. QxP. If a see() value is negative, don't
				 * bother searching the capture since chances are it
				 * won't help our position
				 */
				const int moved    = MOVED(moves[i]);
				const int captured =
								  CAPTURED(moves[i]);

				if (moved != PAWN && Evaluator::piece_value[captured] <
					Evaluator::piece_value[moved])
				{
					if (see(pos, TO(moves[i]), pos.toMove) < 0)
						continue;
				}
			}

			pos.makeMove(moves[i]);
			_node_count++;
			_qnode_count++;

			const int _score = -quiesce(pos, depth+1, -beta, -alpha);

			pos.unMakeMove(moves[i]);

			if (_score > alpha)
			{
				raised_alpha = true;
				best_index = i;
				alpha = _score;
				if (alpha >= beta)
					return beta;
			}
		}

		/*
		 * Save the principal variation up to this node:
		 */
		if (_save_pv && raised_alpha)
		{
			if (0 <= best_index)
				_pv[depth][depth] = moves[best_index];
			else
				_pv[depth][depth] = 0;

			for (register int i = depth+1; i < _depth; i++)
			{
				_pv[depth][i] = _pv[depth+1][i];
			}
		}

		return alpha;
	}

	/**
	 * Static exchange evaluation. This computes the outcome of a sequence
	 * of captures on \a square
	 *
	 * @param [in] Position The position to evaluate
	 * @param [in] square   Square on which to perform the static exchange
	 *                      evaluation
	 * @param[in] to_move   Who captures first
	 *
	 * @return The optimal value of the capture sequence
	 */
	int see(const Position& pos, int square, int to_move) const
	{
		uint64 attackers[2];
		attackers[flip(to_move)] = pos.attacksTo(square, flip(to_move));

		int scores[MAX_PLY];
		int score_index = 1;

		piece_t captured = pos.pieces[square];
		scores[0] =
			 Evaluator::piece_value[captured];

		/*
		 * Bitmap of our defenders:
		 */
		attackers[to_move] = pos.attacksTo(square, to_move);

		/*
		 * Bitmap of the occupied squares. We'll update this as captures
		 * are made:
		 */
		uint64 occupied =
				  pos.occupied[WHITE] | pos.occupied[BLACK];

		/*
		 * Pieces that can X-ray defend:
		 */
		uint64 bishopsQueens =
			pos.bishops[WHITE] | pos.queens[WHITE] |
			pos.bishops[BLACK] | pos.queens[BLACK] ;

		uint64 rooksQueens   = 
			pos.rooks[WHITE]   | pos.queens[WHITE] |
			pos.rooks[BLACK]   | pos.queens[BLACK] ;

		piece_t last_moved = INVALID;

		while (attackers[to_move])
		{
			do
			{
				/*
				 * Check for pawn defenders
				 */
				uint64 piece = attackers[to_move] & pos.pawns[to_move];

				if (piece)
				{
					const int from = _movegen.getMSB64(piece);

					uint64 new_attacker =
						pos.attacksFromBishop(from, occupied)
							& pos.tables.ray_extend[from][square]
							& bishopsQueens;

					_movegen.clearBit64(from, occupied);

					/*
					 * Avoid tagging a bishop or queen sitting on
					 * the capture square:
					 */
					_movegen.clearBit64( square, new_attacker );

					attackers[to_move] &= occupied;

					if (new_attacker & pos.occupied[to_move])
						attackers[to_move] |= new_attacker;
					else
						attackers[flip(to_move)] |= new_attacker;

					last_moved = PAWN;
					break;
				}

				/*
				 * Check for knight defenders
				 */
				piece = attackers[to_move] & pos.knights[to_move];

				if (piece)
				{
					const int from = _movegen.getMSB64(piece);

					_movegen.clearBit64(from, occupied);

								attackers[to_move] &= occupied;

					last_moved = KNIGHT;
					break;
				}

				/*
				 * Check for bishop defenders
				 */
				piece = attackers[to_move] & pos.bishops[to_move];

				if (piece)
				{
					const int from = _movegen.getMSB64(piece);

					uint64 new_attacker =
						pos.attacksFromBishop(from, occupied)
							& pos.tables.ray_extend[from][square]
							& bishopsQueens;

					_movegen.clearBit64(from, occupied);

					/*
					 * Avoid tagging a bishop or queen sitting on
					 * the capture square:
					 */
					_movegen.clearBit64( square, new_attacker );

					attackers[to_move] &= occupied;
					bishopsQueens &= occupied;

					if (new_attacker & pos.occupied[to_move])
						attackers[to_move] |= new_attacker;
					else
						attackers[flip(to_move)] |= new_attacker;

					last_moved = BISHOP;
					break;
				}

				/*
				 * Check for rook defenders
				 */
				piece = attackers[to_move] & pos.rooks[to_move];

				if (piece)
				{
					const int from = _movegen.getMSB64(piece);

					uint64 new_attacker =
						pos.attacksFromRook(from, occupied)
							& pos.tables.ray_extend[from][square]
							& rooksQueens;

					_movegen.clearBit64(from, occupied);

					/*
					 * Avoid tagging a rook or queen sitting on
					 * the capture square:
					 */
					_movegen.clearBit64( square, new_attacker );

					attackers[to_move] &= occupied;
					rooksQueens &= occupied;

					if (new_attacker & pos.occupied[to_move])
						attackers[to_move] |= new_attacker;
					else
						attackers[flip(to_move)] |= new_attacker;

					last_moved = ROOK;
					break;
				}

				/*
				 * Check for queen defenders
				 */
				piece = attackers[to_move] & pos.queens[to_move];

				if (piece)
				{
					const int from = _movegen.getMSB64(piece);

					uint64 new_attacker = 0;
					switch (pos.tables.directions[from][square])
					{
					case ALONG_FILE:
					case ALONG_RANK:
						new_attacker =
							pos.attacksFromRook(from, occupied)
								& rooksQueens;
						break;
					default:
						new_attacker =
							pos.attacksFromBishop(from, occupied)
								& bishopsQueens;
					}

					_movegen.clearBit64(from, occupied);

					/*
					 * Avoid tagging a rook, bishop, or queen sitting
					 * on the capture square:
					 */
					_movegen.clearBit64( square, new_attacker );

					attackers[to_move] &= occupied;
					rooksQueens &= occupied;
						 bishopsQueens &= occupied;

					new_attacker &=
							 pos.tables.ray_extend[from][square];

					if (new_attacker & pos.occupied[to_move])
						attackers[ to_move ] |= new_attacker;
					else
					{
						attackers[flip(to_move)] |= new_attacker;
					}

					last_moved = QUEEN;
					break;
				}

				/*
				 * Check for king defenders
				 */
				piece = attackers[to_move] & pos.kings[to_move];

				if (piece)
				{
					const int from = pos.kingSq[to_move];

					_movegen.clearBit64( from,occupied );

								 attackers[to_move] &= occupied;
					last_moved = KING;
				}

			} while (0);

#if defined(DEBUG_SEE)
			std::cout << (to_move == WHITE ? "WHITE" : "BLACK")
				<< "[" << score_index
				<< "]: "
				<< Util::piece2str(last_moved)
				<< std::endl;
#endif
			to_move = flip(to_move);

			scores[score_index] =
				Evaluator::piece_value[last_moved]
					- scores[score_index-1];
			score_index += 1;
		}

		/*
		 * Now that we've "played" through all the captures,
		 * compute the optimal score via negamax
		 * propagation of the best score up to the root of
		 * the tree, i.e. score[0]. This tree looks like a
		 * binary search tree where at every node we either
		 * capture or not
		 */
		for (int i = score_index-2; i > 0; --i)
        	scores[i-1] = -_max(-scores[i-1], scores[i]);

		return scores[0];
	}

	int _search(Position& pos, int depth, int alpha, int beta)
	{
		uint32 moves[MAX_MOVES];
		uint32* end;

		/*
		 * Forward this position to quiesce() after we've hit
		 * the search limit:
		 */
		if (depth > _depth)
			return quiesce(pos, depth, alpha, beta);

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
			return in_check ?
					((-MATE_SCORE) * (MAX_PLY-depth)) : 0;
		}

		int best_index = -1;

		for (register int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);
			_node_count++;

			const int score = -_search( pos, depth+1, -beta, -alpha);

			pos.unMakeMove(moves[i]);

			if (beta <= score)
				return beta;

			if (score > alpha)
			{
				best_index = i;
				alpha = score;
			}
		}

		/*
		 * Save the principal variation up to this node:
		 */
		if (_save_pv && 0 <= best_index)
		{
			_pv[depth][depth] = moves[best_index];

			for (register int i = depth+1; i < _depth; i++)
			{
				_pv[depth][i] = _pv[depth+1][i];
			}
		}

		return alpha;
	}
};

#endif
