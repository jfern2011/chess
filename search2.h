#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "movegen2.h"
#include "StateMachine2.h"

class Search
{

public:

	/**
	 * Stores the results of the previous search
	 */
	struct SearchData
	{
		SearchData()
			: best_move(0), score(0)
		{
		}

		int best_move;
		int score;
	};

	Search(const MoveGen& movegen);

	virtual ~Search();

	const SearchData& get_search_data() const;

	virtual bool search(const Position& pos)
		= 0;

protected:

	SearchData _data;

	const MoveGen& _movegen;
};

class PvSearch : public Search
{
	class InterruptHandler
	{

	public:

		InterruptHandler(StateMachine& state_machine)
			: _state_machine(state_machine)
		{
		}

		~InterruptHandler()
		{
		}

		bool abort()
		{
			/*
			 * Poll the command interface for user inputs, which will
			 * cause transition requests to flow into the state
			 * machine. Don't print abort messages on error; doing so 
			 * may just send high-rate spam to standard output
			 */
			if (!_state_machine.poll())
				return false;

			return _state_machine.get_current_state()
					!= StateMachine::searching;
		}

	private:

		StateMachine&
			_state_machine;

	};

public:

	PvSearch(const MoveGen& movegen,
		     StateMachine& state_machine,
		     Logger& logger,
		     const DataTables& tables);

	~PvSearch();

	bool init();

	bool is_mated(int to_move) const;

	int quiesce(Position& pos, int depth, int alpha, int beta);

	bool search(const Position& master);

	int see(Position& pos, int square, int to_move, int move=0) const;

private:

	int _search(Position& pos, int depth, int alpha, int beta,
				bool do_null);

	int _search_moves(Position& pos, int* moves, size_t n_moves,
					  int& alpha, int beta, int depth, bool do_null,
					  int& best);

	bool _abort_requested;

	/**
	 * The best move returned by the last search iteration
	 */
	int _best_move;

	BUFFER(int, _current_move, MAX_PLY);

	int _depth;

	int64 _input_check_delay;

	InterruptHandler
		_interrupt_handler;

	bool _is_init;

	Logger& _logger;

	const std::string _name;

	int64 _next_input_check;
	int64 _node_count;

	BUFFER(int, _pv, MAX_PLY, MAX_PLY);

	int64 _qnode_count;

	/**
	 * The optimal score returned by the last search iteration
	 */
	int _search_score;

	const DataTables&
		_tables;
};

inline int PvSearch::quiesce(Position& pos, int depth, int alpha,
							 int beta)
{
	const int to_move   = pos.get_turn();
	const bool in_check = pos.in_check(to_move);

	int moves[MAX_MOVES];
	size_t n_moves;

	if (in_check)
	{
		n_moves =
		   _movegen.generate_check_evasions(pos, to_move, moves);

		/*
		 * Add a penalty to the mate score the encourage
		 * mates closer to the root:
		 */
		if (n_moves == 0)
			return depth - MATE_SCORE;
	}

	/*
	 * Compute an initial score for this position:
	 */
	const int sign = pos.get_turn() == WHITE ? 1 : -1;
	const int score =
		sign * pos.get_material();

	/*
	 * Check if we can "fail-high." Not sure if this is correct for
	 * zugzwang positions...
	 */
	if (score >= beta)
		return beta;

	if (alpha < score) alpha = score;

	if (!in_check)
		n_moves = _movegen.generate_captures(pos,
			to_move, moves );

	/*
	 * Return the heuristic value of this position if
	 * no captures are left:
	 */
	if (n_moves == 0 || MAX_PLY <= depth)
		return score;

	/*
	 * Sort the capture list. Captures are generated starting with
	 * pawns, knights/bishops, rooks, queens, and finally kings.
	 * This increases the likelihood that the captures are already
	 * sorted and bubble_sort() runs in O(n)
	 */
	Util::bubble_sort(moves, n_moves);

	int best_index = -1;

	for (register int i = 0; i < n_moves; i++)
	{
		if (!in_check)
		{
			const int captured = CAPTURED(moves[i]);
			const int moved = MOVED(moves[i]);

			/*
			 * Perform a see() on captures that might be losing,
			 * e.g. QxP. If a see() value is negative, don't
			 * bother searching the capture since chances are it
			 * won't help our position
			 */
			if (moved != PAWN &&
				piece_value[captured] < piece_value[moved])
			{
				if ( see(pos, TO(moves[i]), pos.get_turn()) < 0 )
					continue;
			}
		}

		pos.make_move(moves[i]);
		_node_count++;
		_qnode_count++;

		const int _score = -quiesce(pos, depth+1, -beta, -alpha);

		pos.unmake_move(moves[i]);

		if (_score > alpha)
		{
			best_index = i;
			alpha = _score;
			if (alpha >= beta)
				return beta;
		}
	}

	/*
	 * Save the principal variation up to this node:
	 *
	if (_save_pv && raised_alpha)
	{
		if (0 <= best_index)
			savePV(depth, moves[best_index]);
		else
			savePV(depth, 0);
	}*/

	return alpha;
}

/**
 * Static exchange evaluation. This computes the outcome of a sequence
 * of captures on \a square
 *
 * Note: This can also be used to determine if it is safe to move to
 *       \a square
 *
 * @param [in] Position The position to evaluate
 * @param [in] square   Square on which to perform the static exchange
 *                      evaluation
 * @param [in] to_move  Whose turn it is
 * @param [in] move     See if it is safe to play this move. If a null
 *                      move, it is unused
 *
 * @return The optimal value of the capture sequence
 */
inline int PvSearch::see(Position& pos, int square, int to_move,
	int move) const
{
	int scores[MAX_PLY];
	int score_index = 1;

	scores[0] =  piece_value[ pos.piece_on(square) ];

	if (move)
	{
		pos.make_move(move); to_move = flip(to_move);
	}

	uint64 attackers[2];
	attackers[flip(to_move)]= pos.attacks_to(square, flip(to_move));

	/*
	 * Bitmap of our defenders:
	 */
	attackers[to_move] =
		 pos.attacks_to(square, to_move);

	/*
	 * Bitmap of the occupied squares. We'll update this as captures
	 * are made:
	 */
	uint64 occupied = pos.get_occupied();

	/*
	 * Pieces that can X-ray defend:
	 */
	uint64 bishopsQueens =
		pos.get_bishops() | pos.get_queens();

	uint64 rooksQueens   =
		pos.get_rooks()   | pos.get_queens();

	piece_t last_moved = INVALID;

	while (attackers[to_move])
	{
		do
		{
			/*
			 * Check for pawn defenders
			 */
			uint64 piece =
				attackers[ to_move ] & pos.get_pawns(to_move);

			if (piece)
			{
				const int from = Util::msb64(piece);

				uint64 new_attacker =
					pos.attacks_from_bishop(from, occupied)
							& _tables.ray_extend[from][square]
							& bishopsQueens;

				Util::clear_bit64(from, occupied);

				/*
				 * Avoid tagging a bishop or queen sitting on
				 * the capture square:
				 */
				Util::clear_bit64(square,new_attacker);

				attackers[to_move] &= occupied;

				if (new_attacker & pos.get_occupied(to_move))
					attackers[to_move] |= new_attacker;
				else
					attackers[flip(to_move)] |= new_attacker;

				last_moved = PAWN;
				break;
			}

			/*
			 * Check for knight defenders
			 */
			piece = attackers[to_move] & pos.get_knights(to_move);

			if (piece)
			{
				const int from = Util::msb64(piece);

				Util::clear_bit64(from, occupied);

						attackers[to_move] &= occupied;

				last_moved = KNIGHT;
				break;
			}

			/*
			 * Check for bishop defenders
			 */
			piece = attackers[to_move] & pos.get_bishops(to_move);

			if (piece)
			{
				const int from = Util::msb64(piece);

				uint64 new_attacker =
					pos.attacks_from_bishop(from, occupied)
							& _tables.ray_extend[from][square]
							& bishopsQueens;

				Util::clear_bit64(from, occupied);

				/*
				 * Avoid tagging a bishop or queen sitting on
				 * the capture square:
				 */
				Util::clear_bit64( square, new_attacker );

				attackers[to_move] &= occupied;
				bishopsQueens &= occupied;

				if (new_attacker & pos.get_occupied(to_move))
					attackers[to_move] |= new_attacker;
				else
					attackers[flip(to_move) ] |= new_attacker;

				last_moved = BISHOP;
				break;
			}

			/*
			 * Check for rook defenders
			 */
			piece = attackers[to_move] & pos.get_rooks(to_move);

			if (piece)
			{
				const int from = Util::msb64(piece);

				uint64 new_attacker =
					pos.attacks_from_rook(from, occupied)
							& _tables.ray_extend[from][square]
							& rooksQueens;

				Util::clear_bit64(from, occupied);

				/*
				 * Avoid tagging a rook or queen sitting on
				 * the capture square:
				 */
				Util::clear_bit64( square, new_attacker );

				attackers[to_move] &= occupied;
				rooksQueens &= occupied;

				if (new_attacker & pos.get_occupied(to_move))
					attackers[to_move] |= new_attacker;
				else
					attackers[ flip(to_move) ] |= new_attacker;

				last_moved = ROOK;
				break;
			}

			/*
			 * Check for queen defenders
			 */
			piece = attackers[to_move] & pos.get_queens(to_move);

			if (piece)
			{
				const int from = Util::msb64(piece);

				uint64 new_attacker = 0;
				switch (_tables.directions[from][square])
				{
				case ALONG_FILE:
				case ALONG_RANK:
					new_attacker =
						pos.attacks_from_rook(from, occupied)
							& rooksQueens;
					break;
				default:
					new_attacker =
						pos.attacks_from_bishop(from, occupied)
							& bishopsQueens;
				}

				Util::clear_bit64(from, occupied);

				/*
				 * Avoid tagging a rook, bishop, or queen sitting
				 * on the capture square:
				 */
				Util::clear_bit64( square, new_attacker );

				attackers[to_move] &= occupied;
				rooksQueens &= occupied;
					 bishopsQueens &= occupied;

				new_attacker &=
						 _tables.ray_extend[from][square];

				if (new_attacker & pos.get_occupied(to_move))
					attackers[ to_move ] |= new_attacker;
				else
				{
					attackers[flip(to_move) ] |= new_attacker;
				}

				last_moved = QUEEN;
				break;
			}

			/*
			 * Check for king defenders
			 */
			piece = attackers[to_move] & pos.get_kings(to_move);

			if (piece)
			{
				const int from = pos.get_king_square( to_move );

				Util::clear_bit64(from, occupied );

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

    if (move)
	{
		pos.unmake_move(move);
		to_move = flip( to_move );
	}

	return scores[0];
}

/**
 * To do:
 *
 * 1. repetitions
 * 2. PV retrieval
 * 3. null moves
 * 4. SEE losing captures, and order accordingly?
 */
inline int PvSearch::_search(Position& pos, int depth, int alpha,
							 int beta, bool do_null)
{
	if (_next_input_check <= _node_count)
	{
		/*
		 * Check if this search was interrupted, e.g. by a user
		 * commmand:
		 */
		if (_interrupt_handler.abort())
		{
			_abort_requested = true;
			return beta;
		}

		_next_input_check = _node_count
			+ _input_check_delay;
	}

	/*
	 * Forward this position to quiesce() after we have hit our
	 * search limit:
	 */
	if (_depth <= depth)
		return quiesce(pos, depth, alpha, beta);


	const int to_move   = pos.get_turn();
	const bool in_check = pos.in_check(to_move);

	int moves[MAX_MOVES];

	size_t n_moves = 0;
	int best_move  = 0;

	if (in_check)
	{
		n_moves =
			_movegen.generate_check_evasions(pos, to_move, moves);

		if (n_moves == 0)
		{
			/*
			 * Add a penalty to the mate score the encourage
			 * mates closer to the root:
			 */
			return depth - MATE_SCORE;
		}
	}
	else
	{
		n_moves =
			_movegen.generate_captures(pos, to_move, moves);
	}

	if (n_moves > 0)
	{
		Util::bubble_sort(moves, n_moves);

		const int score =
			_search_moves(pos, moves, n_moves,
				  alpha, beta, depth, !in_check, best_move);

		if (beta <= score)
			return beta;
	}

	if (in_check)
		return alpha;

	/*
	 * Search the remaining moves (non-captures):
	 */
	int* non_captures = &moves[n_moves];

	bool captures = n_moves > 0;

	n_moves = _movegen.generate_non_captures(pos, to_move,
		non_captures);

	if (n_moves == 0 && !captures)
		return 0;

	const int score =
		_search_moves(pos, non_captures,
			n_moves, alpha, beta, depth, true, best_move);

	if (beta <= score)
		return beta;

	return alpha;
}

inline int PvSearch::_search_moves(Position& pos, int* moves,
								   size_t n_moves, int& alpha,
								   int beta, int depth,
								   bool do_null, int& best)
{
	for (register int i = 0; i < n_moves; i++)
	{
		const int move = moves[i];

		/*
		 * We'll get some null moves if the move list was previously
		 * purged:
		 */
		if (move == 0) continue;

		pos.make_move(move);
		_node_count++;

		_current_move[depth] = move;

		const int score = -_search(pos,depth+1,-beta,-alpha,do_null);

		pos.unmake_move(move);

		if (beta <= score)
		{
			/*
			 * Save for the hash table:
			 */
			best = move;
			return beta;
		}

		if (score > alpha)
		{
			alpha = score;
			best = move;
		}
	}

	return alpha;
}

#endif
