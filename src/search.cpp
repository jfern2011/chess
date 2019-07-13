#include <algorithm>
#include <cstring>
#include <limits>

#include "clock.h"
#include "search.h"

namespace Chess
{
	/**
	 * Constructor
	 */
	Search::Search(Handle<OutputChannel> channel)
		: _channel(channel), _is_init(false), _multipv(false),
		  _position(), _start_time(0), _stop_time(0)
	{
		_set_defaults();
	}

	/**
	 * Destructor
	 */
	Search::~Search()
	{
	}

	/**
	 * Enable/disable multi-PV mode. Expect inferior
	 * performance with multi-PV enabled, since it forces
	 * the engine to evaluate suboptimal lines of play
	 *
	 * @param[in] value Enable flag
	 */
	void Search::enable_multipv(bool value)
	{
		_multipv = value;
	}

	/**
	 * Grab the principal variation from the most recent
	 * depth iteration
	 *
	 * @note The PV ends when a null move is encountered
	 *
	 * @return The principal variation
	 */
	MoveList Search::get_pv()
	{
		MoveList list; list.init(_pv[0], 0);

		for (size_t i = 0;
			 _pv[0][i] && i < max_ply; i++ )
			list.size++;

		return list;
	}

	/**
	 * Get the number of hash table hits during the last
	 * search
	 *
	 * @return The number of hits
	 */
	size_t Search::hash_hits() const
	{
		return _hash_hits;
	}

	/**
	 * Get the number of hash table misses during the
	 * last search
	 *
	 * @return The number of misses
	 */
	size_t Search::hash_misses() const
	{
		return _hash_misses;
	}

	/**
	 *  Initialize for a new search. This must be called
	 *  prior to every \ref run()
	 *
	 * @param[in] pos The position to search
	 *
	 * @return True on success
	 */
	bool Search::init(Handle<Position> pos)
	{
		if (!pos) return false;

		_position = pos;

		_set_defaults();

		_is_init = true;
		return true;
	}

	/**
	 * Check for repetitions. This is done by comparing Zobrist keys,
	 * with the first comparison being done with the position 4
	 * plies back, since this is the minimum required plies for a
	 * repetition to occur. From there we proceed by decrementing by
	 * two plies at a time until we hit the root (i.e. 2 unmakes)
	 * in order to catch longer repeat sequences
	 *
	 * @param[in] depth The current search depth, in plies
	 *
	 * @return True if detected
	 */
	bool Search::is_repeated(int depth) const
	{
		if (depth <= 3) return false;

		const auto& pos = *_position;

		/*
		 * Get the ply of the last halfmove clock reset, since
		 * we need not search prior to this:
		 */

		const int limit = pos.last_halfmove_reset(depth);

		const uint64 key = pos.get_hash_key();

		for ( int ply = depth-4; ply >= limit; ply -= 2 )
		{
			if ( key == pos.get_hash_key(ply) )
				return true;
		}

		return false;
	}

	/**
	 * Load an entry from the hash table
	 *
	 * @note That this entry should be replaced is indicated by
	 *       HashEntry::type() == EMPTY
	 *
	 * @param[in]  pos    The position, for verifying the returned
	 *                    entry is indeed usable
	 * @param[in]  draft  The number of plies before reaching the
	 *                    search horizon
	 * @param[in]  alpha  The current lower bound
	 * @param[in]  beta   The current upper bound
	 * @param[in]  check  True if we're currently in check
	 * @param[in]  hint   Suggested move to search first
	 * @param[out] avoid  Flag to indicate that the returned entry
	 *                    should NOT be used
	 *
	 * @return The hash entry
	 */
	HashEntry& Search::load(const Position& pos, int draft,
		int alpha, int beta, bool check, int32& hint, bool& avoid)
	{
		const uint64 key = pos.get_hash_key();
		auto& bucket = hash_table[key];

		avoid = false;
		hint  = 0;
		int16 hint_score = -king_value;

		/*
		 * Age all entries except for the specified one
		 */
		auto age_except = [&bucket](size_t ind) {
			for (size_t i = 0; i < bucket.size; i++)
			{
				if (i != ind) bucket.entries[i].ripen();
			}
		};

		auto& oldest = bucket.entries[0];

		for ( size_t i = 0; i < bucket.size; i++ )
		{
			auto& entry = bucket.entries[i];

			if (entry.key == key && entry.draft >= draft)
			{
				switch (entry.type())
				{
				case FAIL_HI:
					if (entry.score >= beta )
					{
						age_except(i); return entry;
					}
					break;
				case FAIL_LO:
					if (entry.score <= alpha)
					{
						age_except(i); return entry;
					}
					break;
				case EXACT:
					/*
					 * We got an exact hit; validate the stored
					 * move is playable
					 */
					if (entry.score >= alpha && entry.score <= beta
						&& MoveGen::validate_move(
							pos, entry.move(), check))
					{
						age_except( i );
						return entry;
					}
				}
			}

			/*
			 * If the move is playable, and its score
			 * increases alpha, we might want to try it
			 * before searching all remaining moves:
			 */
			if (entry.key == key)
			{
				if (entry.score > hint_score &&
					entry.score > alpha      &&
					MoveGen::validate_move(
							pos, entry.move(), check))
				{
					hint_score = entry.score;
					hint = entry.move();
				}
			}

			/*
			 * Keep track of the most elderly entry
			 */
			if (entry.ripen() > oldest.age)
				oldest = entry;
		}

		/*
		 * If the oldest entry is at the age limit, or
		 * we'll reach greater depth, replace it
		 */

		if (oldest.age >= HashEntry::age_limit ||
			oldest.draft < draft)
		{
			oldest.set_type(EMPTY);
			return oldest;
		}

		/*
		 * Otherwise, keep this entry
		 */
		avoid = true;

		return oldest;
	}

	/**
	 * Quiescent search routine. This searches captures,
	 * promotions, and checks only
	 *
	 * @param[in] depth The current depth
	 * @param[in] alpha The search window lower bound
	 * @param[in] beta  The search window upper bound
	 *
	 * @return The score score (relative to us)
	 */
	int16 Search::quiesce(int depth, int16 alpha, int16 beta)
	{
		Position& pos = *_position;

		const player_t to_move = pos.get_turn();
		const bool in_check = pos.in_check(to_move);

		const auto & tables = DataTables::get();

		BUFFER(int32, moves, max_moves);
		size_t n_moves;

		if (in_check)
		{
			n_moves = MoveGen::generate_check_evasions(
				pos, moves);

			if (n_moves == 0)
			{
				/*
				 * Mark the end of this variation:
				 */
				save_pv(depth, 0);

				/*
			 	 * Add a small penalty to the mate score to encourage
			 	 * mates closer to the root:
			 	 */
				return depth - king_value;
			}
		}

		/*
		 * Get an initial score for this position:
		 */
		const int16 score =
			tables.sign[pos.get_turn()] * evaluate(pos);

		/*
		 * Check if we can fail-high; not sure if this is correct for
		 * zugzwang positions...
		 */
		if (score >= beta)
		{
			_fail_hi++;

			return beta;
		}

		if (alpha < score) alpha = score;

		if (!in_check)
			n_moves = MoveGen::generate_captures(
				pos, moves);

		/*
		 * Return the heuristic value of this position if
		 * no captures are left:
		 */
		if (n_moves == 0 || static_cast<int>(max_ply) <= depth)
		{
			_exact++;

			save_pv(depth, 0);
			return score;
		}

		SelectionSort sort;
			sort.init(moves, n_moves);

		int best_move = 0;
		int32 move;

		while (sort.next(move, [](int32 mv1, int32 mv2) {
				return Chess::score( mv1 ) - Chess::score( mv2 );
			}))
		{
			if (!in_check)
			{
				/*
				 * If this is a promotion, make the move and run a
				 * see() on the "to" square. If we can't promote the
				 * pawn without it getting captured, don't bother
				 * searching this move
				 */
				if (extract_captured(move) == piece_t::empty &&
					extract_promote (move) != piece_t::empty)
				{
					pos.make_move  (move);
					const int score = see(pos, pos.get_turn(),
										  extract_to(move));
					pos.unmake_move(move);

					if ( score > 0 )
						continue;
				}

				/*
				 * Perform a see() on captures that might be losing,
				 * e.g. QxP. If a see() value is negative, don't
				 * bother searching the capture since chances are it
				 * won't help our position
				 */
				else if (tables.piece_value[extract_captured(move)]
					< tables.piece_value[extract_moved(move)])
				{
					if (see(pos, pos.get_turn(), extract_to(move))
						< 0) continue;
				}
			}

			pos.make_move(move);

			_node_count++; _qnode_count++;

			const int score =
				-quiesce( depth+1, -beta, -alpha );

			pos.unmake_move(move);

			if (score >= beta)
			{
				_fail_hi++;

				return beta;
			}

			if ( score > alpha )
			{
				best_move = move;
				alpha = score;
			}
		}

		_exact++;

		save_pv(depth, best_move);

		return alpha;
	}

	/**
	 * Run the search algorithm
	 *
	 * @param[in]  timeout The maximum number of milliseconds
	 *                     to run the search for
	 * @param[in]  depth   The maximum iteration depth
	 * @param[out] best    The best move
	 *
	 * @return The search score (relative to us)
	 */
	int16 Search::run(int timeout, int depth, int32 best)
	{
		if (!_is_init)
		{
			return std::numeric_limits<int16>::max();
		}

		_set_defaults(); _is_init = false;

		lines.resize(_multipv ? 10 : 1); // Number of lines

		auto& channel = *_channel;

		int16 score = -king_value;

		_position->generate_hash(hash_table.size());

		_start_time = Clock::get_monotonic_time();
		_stop_time  =
			_start_time + ((int64)timeout) * 1000000;

		int16 prev_score = -king_value-1;

		while (_iteration_depth <= depth)
		{
			if ( !_multipv )
			{
				if (false)//prev_score >= -king_value) // aspiration search
				{
					int16 alpha = prev_score - 10;
					int16 beta  = prev_score + 10;
					while (true)
					{
						score = search( 0, alpha, beta, true );
						if (score >= beta)
							beta  += (score - beta ) + 10;
						else if (score <= alpha)
							alpha -= (alpha - score) + 10;
						else
							break;
					}
				}
				else
					score = search( 0, -king_value, king_value, true );

				if (_abort_search) break;

				lines.insert(get_pv(), score);

				{
					Position temp(*_position);
					const int moveN =
						_position->get_fullmove_number();

					std::string score_s;
					AbortIfNot(Util::to_string(score, score_s)
						, false);

					channel << score_s + " --> "
						<< Variation::format(
							lines[0], temp, moveN )
						<< std::string("\n");
				}

				prev_score = score;
			}
			else
			{
				// Multi-PV mode enabled
				score = search_root();
			}

			// Back up the PV from this iteration

			for (uint32 i = 0; _pv[0][i] && i < max_ply; i++)
			{
				_saved_pv[i] = _pv[0][i];
			}

			_iteration_depth++;
			lines.clear();
		}

		double pct_fh = _fail_hi * 100.0 / _node_count;
		double pct_fl = _fail_lo * 100.0 / _node_count;
		double pct_ex = _exact   * 100.0 / _node_count;

		char pct_fh_c[64];
		char pct_fl_c[64];
		char pct_ex_c[64];

		std::snprintf(pct_fh_c, 64, "%0.2f", pct_fh);
		std::snprintf(pct_fl_c, 64, "%0.2f", pct_fl);
		std::snprintf(pct_ex_c, 64, "%0.2f", pct_ex);

		std::string pct_fh_s(pct_fh_c);
		std::string pct_fl_s(pct_fl_c);
		std::string pct_ex_s(pct_ex_c);

		channel << "nodes = "
				<< _node_count
				<< ", quiesce = "
				<< _qnode_count
				<< ", reps = "
				<< _reps
				<< ", NMR = "
				<< _nmr
				<< ", exact = "
				<< _exact
				<< " (" << pct_ex_s << "%)"
				<< ", fh = "
				<< _fail_hi
				<< " (" << pct_fh_s << "%)"
				<< ", fl = "
				<< _fail_lo
				<< " (" << pct_fl_s << "%)"
				<< "\nkhits = "
				<< _killer_hits
				<< ", cmhits = "
				<< _counter_move_hits
				<< ", hhits = "
				<< _history_hits
				<< "\n";

		return score;
	}

	/**
	 * Back up the principal variation from the given depth
	 *
	 * @param [in] depth The starting depth
	 * @param [in] move  The move to save at depth \a depth
	 */
	void Search::save_pv(uint32 depth, int move)
	{
		if (depth < max_ply)
		{
			_pv[depth][depth] = move;

			// Null move signals the end of a variation:
			if (move == 0) return;

            for (register uint32 i = depth+1; i < max_ply; i++)
            {
                if ((_pv[depth][i] = _pv[depth+1][i])
                    == 0) break;
            }
		}
	}

	/**
	 * Implements the recursive negamax search algorithm
	 *
	 * @param[in] depth   The current depth
	 * @param[in] alpha   The search window lower bound
	 * @param[in] beta    The search window upper bound
	 * @param[in] do_null True to try a null move
	 *
	 * @return The search score (relative to us)
	 */
	int16 Search::search(int depth, int16 alpha, int16 beta,
		                 bool do_null)
	{
		Position& pos = *_position;

		if (_next_abort_check <= _node_count)
		{
			if (_stop_time <= Clock::get_monotonic_time())
			{
				_abort_search = true;
				return beta;
			}

			// Check for timeouts once per second

			const int64 nps = _node_count / _start_time;
			_next_abort_check = _node_count + nps;
		}

		const player_t side = pos.get_turn();

		const bool in_check = pos.in_check( side );

		/*
		 * First, check for draw by repetition
		 */

		if (is_repeated(depth))
		{
			_reps++;
			if (0 < beta) save_pv( depth, 0 );
			return 0;
		}

		/*
		 * Check if we've hashed this position
		 */

		const auto draft =
			std::max( _iteration_depth - depth, 0);

		bool avoid; int32 hint;
		auto& entry = load(pos, draft, alpha, beta,
			in_check, hint, avoid);

		if (!avoid)
		{
			// Thanks, hash table!

			/**
			 * @todo PV gets cut off due to hits
			 */
#if 0
			if (entry.type() == EXACT)
				save_pv(depth, 0);//entry.move());

			if (entry.type() != EMPTY)
			{
				_hash_hits++;
				return entry.score;
			}
#else
			if (entry.type() != EMPTY &&
				entry.type() != EXACT)
			{
				_hash_hits++;
				return entry.score;
			}
#endif
		}

		_hash_misses++;

		/*
		 * Initialize this position for storage into the
		 * hash table:
		 */
		if (!avoid)
		{
			entry.age   = 0;
			entry.draft = draft;
			entry.key   = pos.get_hash_key();
		}

		/*
		 * Don't quiece() if we're in check:
		 */
		if (_iteration_depth <= depth && !in_check)
			return quiesce(depth, alpha, beta);

		const int16 init_alpha = alpha;
		int best_move = 0;

		SearchPhase phase;

		/*
		 * Initialize the list of searched moves
		 */

		phase.init<phase_t::pv_move>(pos);

		/*
		 * Try a PV move from the previous search
		 * iteration
		 */
		const int32 pv_move = _saved_pv[depth];

		if ((depth < _iteration_depth-1) &&
			MoveGen::validate_move(pos, pv_move, in_check))
		{
			phase.searched_moves.push_back(pv_move);

			int16 score = search_moves< phase_t::pv_move >(
				phase, alpha, beta, depth, !do_null,
				false, false, best_move);

			if ( beta <= score )
			{
				store(pos.get_hash_key(), _iteration_depth -
					depth, beta, pv_move, FAIL_HI);

				_fail_hi++; return beta;
			}
		}

		bool do_zws = alpha > init_alpha;
#if 0
		else if (!hint)
		{
			/*
			 * Internal iterative deepening. For example,
			 * this might be done on the left-most branch 1
			 * ply deeper than the last iteration depth,
			 * since there's no PV move at that depth yet
			 * (and no hint from the hash table)
			 */
			const int score =
				search(depth+2, alpha, beta, do_null);
		}
#endif
		/*
		 * If probing the hash table returned a move to
		 * try, search that one first
		 */
		if (hint && hint != pv_move)
		{
#ifdef DEBUG_HASH
			{
				int32 moves[max_moves];
				size_t n_moves = 0;

				if (in_check)
				{
					n_moves = MoveGen::generate_check_evasions(
						pos, moves);
					if (n_moves == 0)
					{
						Abort(0, "ERROR ERROR ERROR");
					}
				}
				else
				{
					n_moves = MoveGen::generate_captures(
						pos, moves);
					n_moves += MoveGen::generate_noncaptures(
						pos, &moves[n_moves]);
				}

				bool is_valid = false;
				for (size_t i = 0; i < n_moves; i++)
				{
					if (moves[i] == hint)
					{
						is_valid = true; break;
					}
				}

				if (!is_valid)
				{
					Abort(0, "Bad hint! -> %s", (pos.get_fen() +
						":\n" + print_move(hint)).c_str());
				}
			}
#endif
			phase.searched_moves.push_back(hint);

			int16 score = search_moves< phase_t::hash_move >(
				phase, alpha, beta, depth, !do_null,
				do_zws, false, best_move);

			do_zws = alpha > init_alpha;

			if ( beta <= score )
			{
				if (!avoid)
				{
					entry.score = beta;
					entry.set_move(best_move);
					entry.set_type(FAIL_HI);
				}

				_fail_hi++;

				return beta;
			}
		}

		if (in_check)
		{
			phase.init<phase_t::check_evasions>(pos);

			if (phase.evasions.size == 0)
			{
				/*
				 * Mark the end of this variation:
				 */
				save_pv(depth, 0);

				/*
			 	 * Add a small penalty to the mate score to encourage
			 	 * mates closer to the root:
			 	 */
				return depth - king_value;
			}

			const int16 score = 
				search_moves<phase_t::check_evasions>(phase, alpha,
					beta, depth, false, do_zws, false, best_move);

			if ( beta <= score )
			{
				if (!avoid)
				{
					entry.score = beta;
					entry.set_move(best_move);
					entry.set_type(FAIL_HI);
				}

				_fail_hi++;

				return beta;
			}

			if (alpha > init_alpha)
			{
				if (!avoid)
				{
					entry.score = alpha;
					entry.set_move(best_move);
					entry.set_type(EXACT);
				}

				_exact++;

				save_pv(depth,best_move);
			}
			else
			{
				if (!avoid)
				{
					entry.score = alpha;
					entry.set_move(0); /* move */
					entry.set_type(FAIL_LO);
				}

				_fail_lo++;
			}

			return alpha;
		}

		/*
		 * 0. Try a null move, searching with reduced depth
		 */

		if (do_null)
		{
			const int R = 2 + depth/3;

			/*
			 * Assume (conservatively) we're in zugzwang if we only
			 * have pawns left:
			 */
			bool zugzwang =
				(pos.get_bitboard <piece_t::pawn>(side) |
				 pos.get_bitboard <piece_t::king>(side))
				== pos.get_occupied(side);

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
			if (do_null && !zugzwang
				&& depth+R <= _iteration_depth)
			{
				pos.make_move(0);
				_node_count++;

				const int score =
					-search(depth+R, -beta, -beta+1, false);

				pos.unmake_move(0);

				if (beta <= score)
				{
#if 0
					const int _draft = std::max(
						_iteration_depth - (depth+R-1) , 0);
#endif
					const int _draft = std::max(
						_iteration_depth -depth, 0);

					store(pos.get_hash_key(),
						  _draft, beta, 0, FAIL_HI);

					_fail_hi++; _nmr++;

					return beta;
				}
			}
		}

		/*
		 * 1. Search winning captures
		 */

		phase.init<phase_t::winning_captures>(pos);

		int16 score = search_moves< phase_t::winning_captures >(
			phase, alpha, beta, depth, !do_null, do_zws, false, best_move);

		do_zws = alpha > init_alpha;

		if ( beta <= score )
		{
			if (!avoid)
			{
				entry.score = beta;
				entry.set_move(best_move);
				entry.set_type(FAIL_HI);
			}

			_fail_hi++;

			return beta;
		}

		/*
		 * #. Search SEE-based winning captures
		 */

		phase.init<phase_t::winning_captures2>(pos);

		score = search_moves< phase_t::winning_captures2 >(
			phase, alpha, beta, depth, !do_null, do_zws, false, best_move);

		do_zws = alpha > init_alpha;

		if ( beta <= score )
		{
			if (!avoid)
			{
				entry.score = beta;
				entry.set_move(best_move);
				entry.set_type(FAIL_HI);
			}

			_fail_hi++;

			return beta;
		}

		/*
		 * #. Search killer moves from the current ply
		 */

		phase.init<phase_t::killer_moves>(pos);

		for (int i = 0; i < 2; i++)
		{
			const int32 killer = _killers[depth][i].move;

			if (killer == hint || killer == pv_move)
				continue;

			if (MoveGen::validate_move(pos, killer, in_check))
			{
				phase.killer_moves.push_back(killer);
				score = search_moves< phase_t::killer_moves >(
					phase, alpha, beta, depth, !do_null, do_zws,
					false, best_move);

				do_zws = alpha > init_alpha;

				if ( beta <= score )
				{
					_killers[depth][i].hits++;
					_killer_hits++;
					_fail_hi++;

					return beta;
				}
			}
		}

		/*
		 * #. Search killer moves from 2 plies back
		 */

		if (depth >= 2)
		{
			for (int i = 0; i < 2; i++)
			{
				const int32 killer = _killers[depth-2][i].move;

				if (MoveGen::validate_move(pos, killer, in_check))
				{
					phase.killer_moves.push_back(killer);
					score = search_moves< phase_t::killer_moves >(
						phase, alpha, beta, depth, !do_null, do_zws,
						false, best_move);

					do_zws = alpha > init_alpha;

					if ( beta <= score )
					{
						_killers[depth-2][i].hits++;
						_killer_hits++;
						_fail_hi++;

						return beta;
					}
				}
			}
		}

		/*
		 * #. Search counter-moves
		 */

		phase.init<phase_t::counter_moves>(pos);

		if (depth > 0)
		{
			const piece_t moved =
				extract_moved(_current_move[depth-1]);
			const square_t to   = 
				extract_to   (_current_move[depth-1]);

			for (int i = 0; i < 2; i++)
			{
				const int32 counterMove =
					_counter_moves[pos.get_turn()][moved][to][i].move;

				if (phase.killer_moves.find(counterMove) != -1)
					continue;

				if (MoveGen::validate_move(pos, counterMove, in_check))
				{
					phase.counter_moves.push_back(counterMove);

					score = search_moves< phase_t::counter_moves >(
						phase, alpha, beta, depth, !do_null, do_zws,
						false, best_move);

					do_zws = alpha > init_alpha;

					if ( beta <= score )
					{
						if (!avoid)
						{
							entry.score = beta;
							entry.set_move(best_move);
							entry.set_type(FAIL_HI);
						}

						_counter_moves[pos.get_turn()][
							moved][to][i].hits++;

						_counter_move_hits++;
						_fail_hi++;

						return beta;
					}
				}
			}
		}

		/*
		 * 2. Search remaining non-captures
		 */

		phase.init<phase_t::history_moves>(pos);
		phase.history = &_history;

		score = search_moves< phase_t::history_moves >(
			phase, alpha, beta, depth, !do_null, do_zws, true, best_move);

		do_zws = alpha > init_alpha;

		if ( beta <= score )
		{
			if (!avoid)
			{
				entry.score = beta;
				entry.set_move(best_move);
				entry.set_type(FAIL_HI);
			}

			// Save this killer move

			if (_killers[depth][0].hits >
				_killers[depth][1].hits)
			{
				_killers[depth][1].move = best_move;
			}
			else
			{
				_killers[depth][0].move = best_move;
			}

			// Update the counter-move table

			if (depth > 0)
			{
				const piece_t moved =
					extract_moved(_current_move[depth-1]);
				const square_t to   = 
					extract_to   (_current_move[depth-1]);

				if (_counter_moves[pos.get_turn()][moved][to][0].hits > 
					_counter_moves[pos.get_turn()][moved][to][1].hits)
				{
					_counter_moves[pos.get_turn()][moved][to][1].move
						= best_move;
				}
				else
				{
					_counter_moves[pos.get_turn()][moved][to][0].move
						= best_move;
				}
			}

			// Update the history table

			const square_t from =
				extract_from(_current_move[depth]);
			const square_t to   =
				extract_to  (_current_move[depth]);

			const int draft = _iteration_depth - depth;

			_history.scores[pos.get_turn()][from][to]
				+= draft * draft;

			_history_hits++;

			_fail_hi++;

			return beta;
		}

		/*
		 * 3. If no captures/non-captures are available,
		 *    then it is a draw
		 */
		if (phase.winning_captures.size == 0 &&
			phase.non_captures.size == 0)
		{
			_exact++;

			save_pv(depth, 0); // end of line
			return 0;
		}

		/*
		 * 4. Search losing captures
		 */

		phase.init<phase_t::losing_captures>(pos);

		score = search_moves< phase_t::losing_captures >(
			phase, alpha, beta, depth, !do_null, do_zws, false, best_move);

		do_zws = alpha > init_alpha;

		if ( beta <= score )
		{
			if (!avoid)
			{
				entry.score = beta;
				entry.set_move(best_move);
				entry.set_type(FAIL_HI);
			}

			_fail_hi++;

			return beta;
		}

		if (alpha > init_alpha)
		{
			if (!avoid)
			{
				entry.score = alpha;
				entry.set_move(best_move);
				entry.set_type(EXACT);
			}

			_exact++;

			save_pv(depth,best_move);
		}
		else
		{
			if (!avoid)
			{
				entry.score = alpha;
				entry.set_move(0); /* move */
				entry.set_type(FAIL_LO);
			}

			_fail_lo++;
		}

		return alpha;
	}

	/**
	 * Similar to \ref search(), but for the root node only
	 *
	 * @return The search score (relative to us)
	 */
	int16 Search::search_root()
	{
		Position& pos = *_position;

		const bool in_check =
			pos.in_check(pos.get_turn());

		BUFFER(int32, moves, max_moves );
		size_t n_moves;

		if (in_check)
		{
			n_moves = MoveGen::generate_check_evasions(
				pos, moves);

			if (n_moves == 0)
			{
				save_pv(0, 0); return -king_value;
			}
		}
		else
		{
			n_moves = MoveGen::generate_captures(
				pos, moves);

			n_moves += MoveGen::generate_noncaptures(
				pos, &moves[n_moves]);
		}

		if (n_moves == 0) return 0;

		auto best = std::make_pair<int32,int16>(
						0, king_value+1);

		for (size_t i = 0; i < n_moves; i++)
		{
			const int32 move = moves[i];

			pos.make_move( move );

			const int16 score = search(1, -king_value,
				king_value, !in_check);

			pos.unmake_move(move);

			save_pv(0, move);

			if (score < best.second)
			{
				best.first = move; best.second
					= score;
			}

			lines.insert(get_pv(), -score);
		}

		return -best.second;
	}

	/**
	 * Store a new entry into the hash table
	 *
	 * @param[in] entry The entry to store
	 *
	 * @return True if \a entry was stored, or false if
	 *         we rejected it
	 */
	bool Search::store(const HashEntry& entry)
	{
		auto& bucket = hash_table[ entry.key ];

		/*
		 * 1. If there's an entry with the same hash
		 *    signature, overwrite it. This is done in the
		 *    spirit of Crafty's HashStorePV() logic
		 */

		for ( size_t i = 0; i < bucket.size; i++ )
		{
			auto& temp = bucket.entries[i];
			if (temp.key == entry.key)
			{
				std::memcpy(&temp, &entry, sizeof(entry));
				return true;
			}
		}

		/* 2. If there's an empty slot available, then use
		 *    that one
		 */

		auto& oldest = bucket.entries[0];
		for (size_t i = 0; i < bucket.size; i++)
		{
			auto& temp = bucket.entries[i];

			if (temp.type() == EMPTY
				|| temp.age == HashEntry::age_limit)
			{
				std::memcpy(
				   &temp, &entry, sizeof(HashEntry));
				return true;
			}

			if (oldest.age < temp.age)
				oldest = temp;
		}

		/*
		 * 3. If the oldest entry has a lower draft,
		 *    then replace it
		 */

		if (oldest.draft < entry.draft)
		{
			std::memcpy(&oldest, &entry,
				sizeof(HashEntry));
			return true;
		}

		return false;
	}

	/**
	 * Create and store a new hash table entry
	 *
	 * @param[in] key   The position key
	 * @param[in] draft Depth to which this position was
	 *                  searched
	 * @param[in] score The computed score 
	 * @param[in] move  The move that produced the score
	 * @param[in] type  The node type

	 * @return True on success
	 */
	bool Search::store(uint64 key, uint8 draft, int16 score,
					   int32 move, int type)
	{
		HashEntry entry = { 0, /* entry age */
							draft,
							score,
							0, /* type_move */
							key };

		entry.set_move( move );
		entry.set_type( type );

		return store(entry);
	}

	/**
	 * Set default search values. Used during initialization
	 * and construction
	 */
	void Search::_set_defaults()
	{
		_iteration_depth = 3;
		_node_count = _qnode_count = 0;
		_reps = 0;
		_hash_hits = _hash_misses  = 0;

		_exact = _fail_hi = _fail_lo = 0;
		_nmr = 0;
		_killer_hits = _counter_move_hits = 0;
		_history_hits = 0;

		_next_abort_check = 100000;
		_abort_search = false;

		for (size_t i= 0; i < max_ply; i++)
		{
			_saved_pv[i] = 0;

			_current_move[i] = 0;

			/**
			 * @todo We can re-use killers and counter-moves
			 *       in between searches
			 */
			_killers[i][0] = {};
			_killers[i][1] = {};

			_pv[0][i] = 0;
		}

		for (int i = 0; i < 64; i++)
		{
			for (auto piece : { piece_t::pawn,
								piece_t::knight,
								piece_t::bishop,
								piece_t::rook,
								piece_t::queen,
								piece_t::king })
			{
				_counter_moves[player_t::white][piece][i][0]
					= {};
				_counter_moves[player_t::white][piece][i][1]
					= {};
				_counter_moves[player_t::black][piece][i][0]
					= {};
				_counter_moves[player_t::black][piece][i][1]
					= {};
			}

			for (int j = 0; j < 64; j++)
			{
				_history.scores[player_t::white][i][j]
					= {};
				_history.scores[player_t::white][i][j]
					= {};
				_history.scores[player_t::black][i][j]
					= {};
				_history.scores[player_t::black][i][j]
					= {};
			}
		}

		_lmr_factor = 1;
		_lmr_thresh = 2;

		lines.clear();
	}
}
