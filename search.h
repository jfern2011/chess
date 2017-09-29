#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "clock.h"
#include "cmd.h"
#include <cstring>

#include "eval.h"
#include "HashTable.h"

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
		  _counters_enabled(false),
		  _depth (1),
		  _doing_pv(false),
		  _evaluator(movegen),
		  _failed_high(false),
		  _failed_low(false),
		  _hash_enabled(true),
		  _hash_table(),
		  _history_enabled(true),
		  _input_check_delay(100000),
		  _interrupt_handler(),
		  _killers_enabled(false),
		  _mate_found(false),
		  _mate_plies(MAX_PLY),
		  _move_pairs_enabled(true),
		  _movegen(movegen),
		  _next_input_check(0),
		  _nmr_scale (MAX_PLY),
		  _node_count(0),
		  _qnode_count(0),
		  _quit_requested(false),
		  _reps(0),
		   _save_pv(save_pv),
		  _temp_entry(),
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
	 * Get a flag indicating that the last search failed high
	 *
	 * @return True if the searched failed high
	 */
	bool failed_high() const
	{
		return _failed_high;
	}

	/**
	 * Get a flag indicating that the last search failed low
	 *
	 * @return True if the searched failed low
	 */
	bool failed_low() const
	{
		return _failed_low;
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
				  	<< " (" << q_frac * 100 << "%)" << "\n";
		std::cout << "Hash table    = " << _hash_table.in_use()
			<< "/" << HashTable::TABLE_SIZE
			<< "\n";
		std::cout << "Repetitions   = " << _reps
			<< std::endl;
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
	 * @param[in]  _pos      The current position
	 * @param[out] best_move The best move to play
	 *
	 * @return The score of the position
	 *
	 ******************************************************************
	 */
	int search(const Position& _pos, int& best_move)
	{
		uint32 moves[MAX_MOVES];

		/*
		 * Make a copy of the current (real) position. This is done so
		 * that we can reset the internal ply counter and keep it in
		 * sync with the current search depth. This also helps to keep
		 * the ply from exceeding MAX_PLY
		 */
		Position pos(_pos);
		AbortIfNot(pos.reset(_pos.get_fen(),0),false);

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
		_node_count = _qnode_count = _reps = 0;

		/*
		 * Once we hit _input_check_delay nodes, we'll pause to check
		 * for user input. We could just do this at scheduled time
		 * intervals, but the overhead associated with requesting the
		 * system time at every interior node isn't worth it
		 */
		_next_input_check = _input_check_delay;

		/*
		 * Clear the killer moves list as this becomes stale after
		 * each depth iteration
		 */
		for (register int i = 0; i < 2; i++)
		{
			for (register int j = 0; j < MAX_PLY; j++)
				_killers[j][i] = 0;
		}

		/*
		 * Clear the list of counter moves (or do we need to?):
		 */
		for (register int i = 0; i < 2; i++)
		{
			for (register int j = 0; j < 4096; j++)
			{
				_counter_moves[0][i][j] = 0;
				_counter_moves[1][i][j] = 0;
			}
		}

		for (register int i = 0; i < 2; i++)
		{
			for (register int j = 0; j < 4096; j++)
				_move_pairs[i][j] = 0;
		}

		/*
		 * Clear the history moves. Note that these must start off as
		 * zeros
		 */
		for (register int i = 0; i < 64; i++)
		{
			for (register int j = 0; j < 64; j++)
			{
				_histories[0][i][j] = 0;
				_histories[1][i][j] = 0;
			}
		}

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

			_currentMove[0] = moves[i];

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
	int              _counter_moves[2][2][4096];
	bool             _counters_enabled;
	int              _currentMove[MAX_PLY];
	int              _depth;
	Evaluator        _evaluator;
	bool             _failed_high;
	bool             _failed_low;
	bool             _hash_enabled;
	HashTable        _hash_table;
	bool             _history_enabled;
	int              _histories[2][64][64];

	/**
	 * The number of nodes to search before pausing to check for input
	 */
	int              _input_check_delay;
	CommandInterface _interrupt_handler;
	int              _killers[MAX_PLY][2];
	bool             _killers_enabled;
	bool             _mate_found;
	int              _mate_plies;
	int              _move_pairs[2][4096];
	bool             _move_pairs_enabled;
	const MoveGen&   _movegen;
	int              _next_input_check;
	const int        _nmr_scale;
	uint32           _node_count;
	int              _pv[MAX_PLY][MAX_PLY];
	uint32           _qnode_count;
	bool             _quit_requested;
	int              _reps;
	bool             _save_pv;
	double           _time_used;

	/*
	 * The hash entry corresponding to our position
	 */
	HashEntry        _temp_entry;

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
		 * If this position is repeated, assume it's a draw:
		 */
		if (is_repeat(pos, depth))
		{
			_reps++;

			if (0 < beta && _save_pv)
				savePV(depth, 0);
			return 0;
		}

		/*
		 * Forward this position to quiesce() after we have hit our
		 * search limit:
		 */
		if (_depth <= depth)
			return quiesce( pos, depth, alpha, beta );

		// A record of moves we searched first in out move ordering
		// scheme:
		uint32 black_list[MAX_MOVES];
		int n_listed = 0;

		const bool in_check = pos.inCheck(pos.toMove);
		bool captures = true;
		int best_move = 0, nMoves = 0;

		/*
		 * First, probe the hash table to see if we can immediately
		 * return the result of this position
		 */
		uint32 hash_move = 0;

		if (_hash_enabled)
		{
			int node_type = 0;
			int score = lookup_hash_move(pos, in_check, alpha, beta,
									depth, node_type, do_null);

			if (node_type != 0)
				return score;
		}

		if (in_check)
		{
			end =
			   _movegen.generateCheckEvasions(pos, pos.toMove, moves);
			nMoves = end-moves;

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
		}
		else
		{
			end = _movegen.generateCaptures( pos, pos.toMove, moves );
			nMoves = end-moves;

			if (nMoves == 0)
			{
				/*
				 * No captures are available, let's see if there are
				 * any non-captures:
				 */
				captures = false;
				end =
				 _movegen.generateNonCaptures(pos,pos.toMove,moves);
				 nMoves = end-moves;

				 if (nMoves == 0)
				 {
				 	/*
				 	 * We are not in check but there are no moves left
				 	 * to make, so it's a draw
				 	 */
					if (_save_pv)
						savePV(depth, 0);

					return 0;
				 }
			}

			if (do_null)
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

					_currentMove[depth] = 0;

					const int score =
						-_search(pos, depth+R, -beta, -beta+1, false);

					pos.unMakeMove(0);

					if (beta <= score)
						return beta;
				}
			}
		}

		if (in_check || captures)
		{
			/*
		 	 *  First, search the captures or, if we are in check,
		 	 *  the evasions list:
		 	 */
			bubbleSort( moves, nMoves );

			const int score =
				searchMoves(pos,moves,nMoves,alpha,beta, depth,
							true,best_move);

			if (beta <= score)
			{
				if (_hash_enabled)
				{
					insert_hash_entry(pos,depth,false,best_move,
						FAIL_HI, beta);
				}

				return beta;
			}

			/*
			 * If we're in check then we've searched all possible
			 * evasions, so we're done
			 */
			if (in_check)
			{
				if (_save_pv && best_move > 0)
					savePV( depth,best_move );

				if (_hash_enabled)
				{
					if (best_move > 0)
						insert_hash_entry(pos,
									  depth,
									  false,
									  best_move,
									  PV_NODE,
									  alpha);
					else
						insert_hash_entry(pos,
									  depth,
									  false,
									  0,
									  FAIL_LO,
									  alpha);
				}

				return alpha;
			}
		}

		/*
		 * Next apply the killer move heuristic by trying two killers
		 * at the current ply
		 */
		if (_killers_enabled && depth > 1)
		{
			int move = _killers[depth][0];

			if (!in_list(move, black_list, n_listed)
				 && _movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false, move,
							FAIL_HI, beta);
					}

					return beta;
				}
			}

			move     = _killers[depth][1];

			if (!in_list(move, black_list, n_listed)
				 && _movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false, move,
							FAIL_HI, beta);
					}

					return beta;
				}
			}
		}

		/*
		 * Next apply the killer move heuristic by trying two killers
		 * 2 plies back
		 */
		if (_killers_enabled && depth > 2)
		{
			int move=_killers[depth-2][0];

			if (!in_list(move, black_list, n_listed)
				 && _movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false, move,
							FAIL_HI, beta);
					}

					return beta;
				}
			}

			move    =_killers[depth-2][1];

			if (!in_list(move, black_list, n_listed)
				 && _movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false, move,
							FAIL_HI, beta);
					}

					return beta;
				}
			}
		}

		/*
		 * Next try a couple of counter-moves:
		 */
		if (_counters_enabled && depth > 0)
		{
			const int prev_move =  _currentMove[depth-1] & 0xFFF;
			int move            = 
						_counter_moves[pos.toMove][0][prev_move];

			if (!in_list(move, black_list, n_listed) &&
				_movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						  best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false,
							move, FAIL_HI, beta);
					}

					return beta;
				}
			}

			move      = _counter_moves[pos.toMove][1][prev_move];

			if (!in_list(move, black_list, n_listed) &&
				_movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						  best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false,
							move, FAIL_HI, beta);
					}

					return beta;
				}
			}
		}

		/*
		 * Finally, try move pairs (an idea borrowed from Crafty):
		 */
		if (_move_pairs_enabled && depth > 1)
		{
			const int prev_move =  _currentMove[depth-2] & 0xFFF;
			int move            = 
								 _move_pairs[0][prev_move];

			if (!in_list(move, black_list, n_listed) &&
				_movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						  best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false, move,
							FAIL_HI, beta);
					}

					return beta;
				}
			}

			move = _move_pairs[1][prev_move];

			if (!in_list(move, black_list, n_listed)
				  && _movegen.validateMove(pos, move, in_check))
			{
				const int score =
					searchMove(pos, alpha, beta, depth,
						  best_move, move, black_list, n_listed);

				if (beta <= score)
				{
					if (_hash_enabled)
					{
						insert_hash_entry(pos, depth, false, move,
							FAIL_HI, beta);
					}

					return beta;
				}
			}
		}

		/*
		 *  Search remaining moves, which include non-captures only
		 */
		if (!in_check)
		{
			uint32* nonCaptures;
			if (captures)
			{
				/*
				 * We still need to generate the non-captures list:
				 */
				nonCaptures = end;
				end = _movegen.generateNonCaptures(
							pos,pos.toMove,nonCaptures);
				nMoves =
					end-nonCaptures;
			}
			else
			{
				nonCaptures = moves;
			}

			purge_moves(black_list, n_listed,
							nonCaptures, nMoves);

			int score;

			if (_history_enabled)
			{
				score = search_history(pos,nonCaptures, nMoves, alpha,
						 		beta, depth, best_move);
			}
			else
			{
				score = searchMoves(pos,nonCaptures,nMoves,alpha,beta,
								depth, true, best_move);
			}

			if (beta <= score)
			{
				if (_hash_enabled)
				{
					insert_hash_entry( pos, depth, false,
						best_move, FAIL_HI, beta );
				}

				return beta;
			}
		}

		/*
		 * Save the principal variation up to this node:
		 */
		if (_save_pv && best_move > 0)
		{
			savePV( depth, best_move );
		}

		if (_hash_enabled)
		{
			if (best_move > 0)
				insert_hash_entry(pos,
							  depth,
							  false,
							  best_move,
							  PV_NODE,
							  alpha);
			else
				insert_hash_entry(pos,
							  depth,
							  false,
							  0,
							  FAIL_LO,
							  alpha);
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
	 *  Compare two captures. This is used by quiesce() to sort its list
	 *  of captures
	 *
	 * Captures are compared using the MVV/LVA approach, e.g. PxQ is
	 * ordered before PxR
	 *
	 * @param[in] a The first value
	 * @param[in] b The value to compare the first against
	 *
	 * @return True if \a b is less than or equal to \a a; returns false
	 *         otherwise
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
	 * Check if the specified move already exists in a given move list
	 *
	 * @param[in] move     The move to search for
	 * @param[in] moves    The list of moves
	 * @param[in] n_listed Total number of moves
	 *
	 * @return True if the list includes this move
	 */
	inline bool in_list(int move, uint32* moves, int n_listed) const
	{
		for (register int i = 0; i < n_listed; i++)
		{
			if (move == moves[i]) return true;
		}

		return false;
	}

	/**
	 * Insert a new counter-move into the counter-move database. This is
	 * done after each fail-high
	 *
	 * @param[in] key  The key for this entry (a 12-bit integer encoding
	 *                 from/to squares)
	 * @param[in] move The counter-move to insert
	 * @param[in] side The side who played
	 */
	inline void insert_counter(int key, int move, int side)
	{
		Util::xor_swap<int>(_counter_moves[side][0][key],
							_counter_moves[side][1][key]);
		_counter_moves[side][0][key] = move;
	}

	/**
	 * Decide on whether to insert a new entry into the hash table,
	 * overwriting the old entry if needed
	 */
	inline void insert_hash_entry(Position& pos, int depth,
					  bool do_null, int move, int type, int score)
	{
		const uint64 key = pos.get_hash_key();

		_temp_entry.age       = 0;
		_temp_entry.depth     = _depth -depth;
		_temp_entry.do_null   = do_null;
		_temp_entry.hits      = 0;
		_temp_entry.key       = key;
		_temp_entry.move      = move;
		_temp_entry.node_type = type;
		_temp_entry.score     = score;

		HashEntries& entries=_hash_table[key];

		entries.insert(_temp_entry);
	}

	/**
	 *  Insert a new killer move into the killers database. This is done
	 *  after each fail-high
	 *
	 * @param[in] ply  Insert the killer at this ply
	 * @param[in] move The killer to insert
	 */
	inline void insert_killer(int ply, int move)
	{
		Util::xor_swap<int>(_killers[ply][0], _killers[ply][1]);
		_killers[ply][0] = move;
	}

	/**
	 * Insert a new move in the move pairs database. This is a move that
	 * failed high and is paired with the move played two plies ago,
	 * i.e. it is a continuation
	 *
	 * Borrowed this idea from Crafty :)
	 *
	 * @param[in] key  The key for this entry (a 12-bit integer encoding
	 *                 from/to squares)
	 * @param[in] move The counter-move to insert
	 */
	inline void insert_move_pair(int key, int move)
	{
		Util::xor_swap<int>(_move_pairs[0][key],
							_move_pairs[1][key]);
		_move_pairs[0][key] = move;
	}

	/**
	 * Check for repetitions. This is done by comparing Zobrist keys,
	 * with the first comparison being done with the position 4
	 * plies back, since this is the minimum requires plies for a
	 * repetition to occur. From there we proceed by decrementing by
	 * two plies at a time until we hit the root (i.e. 2 unmakes)
	 * in order to catch longer repeat sequences
	 */
	inline bool is_repeat(const Position& pos, int depth) const
	{
		if (depth > 3)
		{
			const uint64 key = pos.get_hash_key();

			for (register int ply = depth-4; ply >= 0; ply -= 2)
			{
				if (key == pos.get_hash_key(ply))
					return true;
			}
		}

		return false;
	}

	/**
	 * Lookup a move from the hash table
	 *
	 * @param[in]  pos      The current position
	 * @param[in]  in_check Flag indicating the side on move is in check
	 * @param[in]  alpha    Current lower bound
	 * @param[in]  beta     Current upper bound
	 * @param[in]  depth    The current search depth
	 * @param[out] type     The node type of the hash move looked up
	 * @param[out] do_null   False if the null move heuristic previously
	 *                      failed
	 *
	 * @return The hashed score of this position, or zero if no move was
	 *         found (type = 0 in this case)
	 */
	inline int lookup_hash_move(const Position& pos, bool in_check,
								int alpha, int beta, int depth, int& type,
								bool& do_null)
	{
		const uint64 key = pos.get_hash_key();

		HashEntries& entries = _hash_table[ key ];

		for (register int i = 0; i < HashEntries::N_ENTRIES; i++)
		{
			HashEntry& entry = entries[i];

			if (entry.key == key && entry.node_type != 0
				&& entry.depth >= (_depth-depth)
				&& _movegen.validateMove(pos,entry.move,in_check))
			{
				//do_null = entry.do_null;
				type  = entry.node_type;
				switch (entry.node_type)
				{
				case FAIL_HI:
					if (beta  <= entry.score)
					{
						entry.hits++;
						return beta;
					}
					break;
				case FAIL_LO:
					if (entry.score <= alpha)
					{
						entry.hits++;
						return alpha;
					}
					break;
				case PV_NODE:
					// TODO: Save principal variation
					entry.hits++;
					return entry.score;
				}
			}
		}

			type = 0;
		return 0;
	}

	/**
	 * Given a list of generated moves, remove those that are listed in
	 * an exclusion list
	 *
	 * @param[in]     exclude    The list of moves to exclude
	 * @param[in]     n_exclude  Number of exclusions
	 * @param[in,out] moves      The list of generated moves
	 * @param[in]     nMoves     Number of moves
	 *
	 */
	inline void purge_moves(const uint32* exclude, int n_exclude,
							uint32* moves, int nMoves) const
	{
		if (n_exclude == 0)
			return;

		for (register int i = 0, n_found = 0; i < nMoves; i++)
		{
			for (int j = 0; j < n_exclude; j++)
			{
				if (moves[i] == exclude[j])
				{
						moves[i] = 0; n_found++;
					break;
				}
			}

			/*
			 * Iterate through the move list until the current excluded
			 * moves are found:
			 */
			if (n_found == n_exclude)
				return;
		}
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

	/*
	 * Intended for searching the set of quiet moves that are not
	 * killers, counter-moves, or move-pairs
	 *
	 * @param[in]     pos       The current position
	 * @param[in,out] moves     The list of move to search
	 * @param[in]     nMoves    Size of \a moves
	 * @param[in,out] alpha     The current lower bound
	 * @param[in]     beta      The current upper bound
	 * @param[in]     depth     The current search depth
	 * @param[out]    best_move The best move if alpha was raised
	 * @param[in]     do_null   If true, try a null move
	 *
	 * @return The score of this position
	 */
	inline int search_history(Position& pos, uint32* moves, int nMoves,
							  int& alpha, int beta, int depth,
							  int& best_move, bool do_null = true)
	{
		const int to_move = pos.toMove;

		while (true)
		{
			int best_val = MIN_INT32;
			int best_id  = -1;

			/*
			 * 1. Select the move with the highest history score:
			 */
			for (register int i = 0; i < nMoves; i++)
			{
				if (moves[i] == 0)
					continue;

				const int from = FROM(moves[i]);
				const int to   = TO  (moves[i]);

				const int hscore =
						_histories[to_move][from][to];

				if (hscore > best_val || best_id < 0 )
				{
					best_val = hscore;
					best_id  = i;
				}
			}

			/*
			 * 2. If no best move was found, exit
			 */
			if (best_id == -1)
				break;

			/*
			 * 3. Otherwise, search this move and mark it null
			 *    to avoid re-searching it
			 */
			const int move = moves[best_id];

			pos.makeMove(move);
			_node_count++;

			_currentMove[depth] = move;

			const int score =
				-_search(pos,depth+1,-beta,-alpha, do_null);

			pos.unMakeMove(move);

			moves[best_id] = 0;

			if (beta <= score)
			{
				/*
				 * Save this move in the list of counters, killers,
				 * pairs, and histories:
				 */
				if (CAPTURED(move) == INVALID)
				{
					if (_counters_enabled  &&  depth > 0)
					{
						int prev_key =
							_currentMove[depth-1];

						insert_counter(prev_key & 0xFFF,
							move, to_move);
					}

					if (_killers_enabled)
						insert_killer(depth, move);

					if (_move_pairs_enabled && depth > 1)
					{
						int prev_key =
							_currentMove[depth-2];

						insert_move_pair(prev_key &0xFFF,
							move);
					}

					if (_history_enabled)
					{
						const int draft = (_depth - depth);
									
						_histories[to_move][FROM(move)][TO(move)]
							+= draft * draft;
					}
				}

				// Save for the hash table:
				best_move = move;
				return beta;
			}

			if (score > alpha)
			{
				best_move = move;
				alpha = score;
			}
		}

		return alpha;
	}

	/**
	 * Search the given move and add it to a black list, i.e. a list of
	 * moves not to try again
	 *
	 * @param[in]     pos        The current position
	 * @param[in,out] alpha      Lower bound
	 * @param[in]     beta       Upper bound
	 * @param[in]     depth      Current search depth
	 * @param[in,out] best_move  The new best move
	 * @param[in]     move       The move to search
	 * @param[out]    black_list The black list
	 * @param[out]    n_listed   Size of the black list
	 *
	 * @return The score returned after searching
	 */
	inline int searchMove(Position& pos, int& alpha, int beta, int depth,
						int& best_move, int move, uint32* black_list,
						  int& n_listed)
	{
		pos.makeMove(move);
		_node_count++;

		_currentMove[depth] = move;

		const int score = -_search(pos, depth+1,-beta, -alpha, true);

		pos.unMakeMove(move);

		black_list[n_listed++]
			= move;

		if (beta <= score)
			return beta;

		if (score > alpha)
		{
			best_move = move;
			alpha = score;
		}

		return alpha;
	}

	/*
	 * Iterate through a given list of moves, calling _search() on each
	 * one. This is done here to reduce code redundancy
	 *
	 * @param[in]    pos     The current position from which to search
	 * @param[in]    moves   The list of moves to search
	 * @param[in]    nMoves  The number of moves in this list
	 * @param[in,out] alpha  The current lower bound
	 * @param[in]    beta    The current upper bound
	 * @param[in]    depth   The current search depth
	 * @param[in]    do_null Flag that indicates that we're free to try
	 *                       a null move
	 * @param[out] best_move The best move
	 *
	 * @return The search score
	 */
	inline int searchMoves(Position& pos, uint32* moves, int nMoves,
						   int& alpha, int beta,
							 int depth, int do_null, int& best_move)
	{
		for (register int i = 0; i < nMoves; i++)
		{
			const int move = moves[i];

			/*
			 * We'll get some null moves if the move list was previously
			 * purged:
			 */
			if (move == 0) continue;

			pos.makeMove(move);
			_node_count++;

			_currentMove[depth] = move;

			const int score =
				-_search(pos, depth+1, -beta, -alpha, do_null);

			pos.unMakeMove(move);

			if (beta <= score)
			{
				if (CAPTURED(move) == INVALID)
				{
					if (_counters_enabled  &&  depth > 0)
					{
						int prev_key =
							_currentMove[depth-1];

						insert_counter(prev_key & 0xFFF,move,pos.toMove);
					}

					if (_killers_enabled)
						insert_killer(depth, move);

					if (_move_pairs_enabled && depth > 1)
					{
						int prev_key =
							_currentMove[depth-2];

						insert_move_pair(prev_key &0xFFF,
							move);
					}

					if (_history_enabled)
					{
						const int draft = (_depth - depth);
									
						_histories[pos.toMove ][ FROM(move) ][ TO(move) ]
							+= draft * draft;
					}
				}

				// Save for the hash table:
				best_move = move;
				return beta;
			}

			if (score > alpha)
			{
				best_move = move;
				alpha = score;
			}
		}

		return alpha;
	}

	/**
	 * Static exchange evaluation. This computes the outcome of a sequence
	 * of captures on \a square
	 *
	 * Note: This can also be used to check if it is safe to move to a
	 * particular square, except for the case of a pawn. For example,
	 * playing a3 from the following position results in the loss of
	 * White's pawn (or more, for promotions), but see() thinks it is
	 * safe. We may decide to fix this at some point, or just stick
	 * with the caveat:
	 *
	 * 4k3/1P5p/8/1nP1PpP1/8/8/P2r4/4K2R w K - 0 1
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
		 * capture or choose not to
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
