#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "clock.h"
#include "cmd.h"
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
		: _abort_requested(false),
		  _base_R(3),
		  _depth (1),
		  _evaluator(movegen),
		  _input_check_delay(100000),
		  _interrupt_handler(),
		  _mate_found(false),
		  _mate_plies(MAX_PLY),
		  _movegen(movegen),
		  _next_input_check(0),
		  _nmr_scale (6),
		  _node_count(0),
		  _qnode_count(0),
		  _quit_requested(false),
		   _save_pv(save_pv),
		  _time_used(0)
	{
	}

	/**
	 * Destructor
	 */
	~Node()
	{
	}

	/**
	 * Check if an "abort" command was sent
	 *
	 * @return True if this command was issued
	 */
	bool abort_requested() const
	{
		return _abort_requested;
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
	 * Initialize the interrupt handler. This is used to handle command
	 * requests during an active search
	 *
	 * @return True on success
	 */
	bool init()
	{
		AbortIf(_interrupt_handler.install("abort",
						*this, &Node::set_abort) < 0, false);
		AbortIf(_interrupt_handler.install("quit",
						*this, &Node::set_quit ) < 0, false);

		return true;
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
	 * Print statistics from the previous search to standard output
	 */
	void print_stats() const
	{
		const double q_frac =
			static_cast<double>(_qnode_count) / _node_count;

		std::cout << "Time used (s) = " << _time_used  << "\n";
		std::cout << "Nodes         = " << _node_count << "\n";
		std::cout << "Quiesce       = " << _qnode_count
				  	<< " (" << q_frac * 100 << "%)" << std::endl;
	}

	/**
	 * Check whether a "quit" command was sent
	 *
	 * @return True if this command was issued
	 */
	bool quit_requested() const
	{
		return _quit_requested;
	}

	/**
	 ******************************************************************
	 *
	 * Search for the best move from the given position
	 *
	 * @param[in]  pos       The current position
	 * @param[out] best_move The best move to play
	 *
	 * @return The score of the position
	 *
	 ******************************************************************
	 */
	int search(Position& pos, int& best_move)
	{
		uint32 moves[MAX_MOVES];

		const int sign = pos.toMove == WHITE ? 1 : -1;

		uint32* end;

		const int64 start_time =
						  Clock::get_monotonic_time();

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
		 * Once we hit _input_check_delay nodes, we'll pause to check
		 * for user input. We could just do this at scheduled time
		 * intervals, but the overhead associated with requesting the
		 * system time at every interior node isn't worth it
		 */
		_next_input_check = _input_check_delay;

		/*
		 * Clear the principal variation. Note that PV read-out ends
		 * when we hit the first null move
		 */
		clearPV();

		for (register int i = 0; i < nMoves ; i++)
		{
			bool raised_alpha = false;

			pos.makeMove(moves[i]);
			_node_count++;

			_abort_requested = _quit_requested = false;

			if (pos.toMove == flip(WHITE))
			{
				const int temp =
					-_search( pos, 1, init_alpha, init_beta, true );

				if (temp > score)
				{
					best_move= moves[i];
					score = temp;
					raised_alpha = true;
				}
			}
			else
			{
				const int temp =
					 _search( pos, 1, init_alpha, init_beta, true );

				if (temp < score)
				{
					best_move= moves[i];
					score = temp;
					raised_alpha = true;
				}
			}

			pos.unMakeMove(moves[i]);

			if (_abort_requested || _quit_requested)
				return 0;

			/*
			 * Save the principal variation up to this node:
			 */
			if (_save_pv && (raised_alpha || i == 0))
			{
				savePV(0, moves[i]);
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

		const int64 stop_time =
						 Clock::get_monotonic_time();

		_time_used =
			static_cast<double>(stop_time-start_time)
				/ NS_PER_SEC;

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

	/**
	 * Set the delay, in nodes, between checks for user input
	 *
	 * @param[in] delay Desired delay
	 */
	void set_input_check_delay(int delay)
	{
		_input_check_delay = delay;
	}

private:

	bool             _abort_requested;

	/**
	 * The base value of the depth reduction parameter used by the null
	 * move heuristic
	 */
	const int        _base_R;
	int              _depth;
	Evaluator        _evaluator;

	/**
	 * The number of nodes to search before pausing to check for input
	 */
	int              _input_check_delay;
	CommandInterface _interrupt_handler;
	bool             _mate_found;
	int              _mate_plies;
	const MoveGen&   _movegen;
	int              _next_input_check;
	const int        _nmr_scale;
	uint32           _node_count;
	int              _pv[MAX_PLY][MAX_PLY];
	uint32           _qnode_count;
	bool             _quit_requested;
	bool             _save_pv;
	double           _time_used;

	/**
	 * Routine that implements the negamax alpha-beta search algorithm
	 * from an interior node
	 *
	 * @param[in] pos     The position at this depth
	 * @param[in] depth   Current search depth
	 * @param[in] alpha   Lower bound on the value of this position
	 * @param[in] beta    Upper bound on the value of this position
	 * @param[in] do_null If true, try a null move
	 *
	 * @return The score of this position if it falls within the given
	 *         bounds, \a alpha if the score is less than the lower
	 *         bound or \a beta if the score is greater than the upper
	 *         bound
	 */
	int _search(Position& pos, int depth, int alpha, int beta,
				bool do_null)
	{
		uint32 moves[MAX_MOVES];
		uint32* end;

		/*
		 * Check if a search abort was requested. If true, return beta
		 * so that the calling node produces a cutoff and returns as
		 * well. Otherwise, check if it is time to poll the input file
		 * descriptor for commands
		 */
		if (_abort_requested || _quit_requested)
			return beta;
		else if (_node_count >= _next_input_check)
		{
			_interrupt_handler.poll();
				_next_input_check = _node_count + _input_check_delay;
		}

		/*
		 * Forward this position to quiesce() after we have hit our
		 * search limit:
		 */
		if (_depth <= depth)
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
				savePV(depth, 0);

			// Scale the mate score to favor checkmates
			// in fewer moves:
			return in_check ?
					((-MATE_SCORE) * (MAX_PLY-depth)) : 0;
		}
		else if (do_null && !in_check)
		{
			const int R = _base_R + depth/_nmr_scale;

			/*
			 * Assume (conservatively) we're in zugzwang if we only
			 * have pawns left:
			 */
			bool zugzwang =
				(pos.pawns[pos.toMove] | pos.kings[pos.toMove])
				== pos.occupied[pos.toMove];

			/*
			 * Null move heuristic. Since we're not in check (and
			 * not in zugzwang), try passing this turn (e.g. the
			 * opponent gets two turns in a row). If we can still
			 * raise alpha enough to get a cutoff, then chances
			 * are we'll definitely get a cutoff by searching in
			 * the usual way. Note that we initially reduce by two
			 * plies, and further reduce for every increase in
			 * depth by three plies:
			 */
			if (do_null && !zugzwang && depth+R < _depth)
			{
				pos.makeMove(0);
				_node_count++;

				const int score =
					-_search(pos, depth+R, -beta, -alpha, false );

				pos.unMakeMove(0);

				if (beta <= score)
					return beta;
			}
		}

		int best_index = -1;

		for (register int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);
			_node_count++;

			const int score =
				-_search(pos,depth+1,-beta, -alpha, true);

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
			savePV(depth, moves[best_index]);
		}

		return alpha;
	}

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
	 * Clear the principal variation (i.e. fill the PV with null moves)
	 */
	inline void clearPV()
	{
		for (register int i = 0; i < MAX_PLY; i++)
			for (register int j = 0; j < MAX_PLY; j++)
				_pv[i][j] = 0;
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
			gain_a =
				piece_value[CAPTURED(a)] - piece_value[MOVED(a)];

		if (PROMOTE(b) && !CAPTURED(b))
			gain_b = 0;
		else
			gain_b =
				piece_value[CAPTURED(b)] - piece_value[MOVED(b)];

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
					savePV(depth, 0);

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

		if (alpha < score)
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
				savePV(depth, 0);
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

				if (moved != PAWN &&
					piece_value[captured] < piece_value[moved])
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
				savePV(depth, moves[best_index]);
			else
				savePV(depth, 0);
		}

		return alpha;
	}

	/**
	 * Save the principal variation, starting at the specified depth
	 *
	 * @param[in] depth The current search depth
	 * @param[in] move  The move to save at depth \a depth
	 */
	inline void savePV(int depth, int move)
	{
		if (depth < MAX_PLY)
		{
			_pv[depth][depth] = move;

			// Null move signals the end of a variation:
			if (move == 0)
				return;
		}

		if (depth+1 < MAX_PLY)
		{
			for (register int i= depth+1; i < MAX_PLY; i++)
			{
				if ((_pv[depth][i] = _pv[depth+1][i]) == 0)
					break;
			}
		}
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
			piece_value[captured];

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
				piece_value[last_moved]
				-scores[score_index-1];
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

	bool set_abort(const std::string& args)
	{
		_abort_requested = true;
		return true;
	}

	bool set_quit (const std::string& args)
	{
		_quit_requested  = true;
		return true;
	}
};

#endif
