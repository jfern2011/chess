#ifndef __SEARCH_H__
#define __SEARCH_H__

#include "HashTable.h"
#include "eval.h"
#include "MoveGen4.h"
#include "OutputChannel.h"
#include "see.h"
#include "SearchPhase.h"
#include "Variation.h"

namespace Chess
{
	/**
	 * Alpha-beta negamax search algorithm
	 */
	class Search final
	{

	public:

		Search(Handle<OutputChannel> channel);

		~Search();

		void enable_multipv(bool value);

		MoveList get_pv();

		size_t hash_hits() const;

		size_t hash_misses() const;

		bool init(Handle<Position> pos);

		bool is_repeated(int depth) const;

		HashEntry& load(const Position& pos, int draft, int alpha,
						int beta, bool check, int32& hint,
						bool& avoid);

		int16 quiesce(int depth, int16 alpha, int16 beta );

		int16 run(int timeout, int depth, int32 best);

		void save_pv(int depth, int move);

		int16 search( int depth, int16 alpha, int16 beta,
		              bool do_null );

		int16 search_root();

		template <phase_t P>
		int16 search_moves(SearchPhase& phase,
						   int16& alpha, int16 beta,
						   int depth, bool do_null,
						   bool do_zws, int& best);

		bool store(const HashEntry& entry);

		bool store(uint64 key, uint8 draft, int16 score,
				   int32 move, int type);

		Variation lines;

		HashTable<1> hash_table;

	private:

		void _set_defaults();

		/**
		 * If true, then abort the currently active search
		 */
		bool _abort_search;

		/**
		 *  Channel through which to send outputs
		 */
		Handle<OutputChannel> _channel;

		/**
		 *  The number of PV nodes
		 */
		uint64 _exact;

		/**
		 * The number of fail-high nodes
		 */
		uint64 _fail_hi;

		/**
		 *  The number of fail-low nodes
		 */
		uint64 _fail_lo;

		/**
		 * The number of hash table hits
		 */
		size_t _hash_hits;

		/**
		 * The number of hash table misses
		 */
		size_t _hash_misses;

		/**
		 * Indicates if the search is initialized
		 */
		bool _is_init;

		/**
		 * Iterative deepening iteration depth
		 */
		int _iteration_depth;

		/**
		 * Multi-PV mode enabled flag
		 */
		bool _multipv;

		/**
		 * The next time to check for a search
		 * abort, in number of nodes
		 */
		int64 _next_abort_check;

		/**
		 * Number of null move reductions
		 */
		uint64 _nmr;

		/**
		 * The number of nodes visited, including
		 * quiescent nodes
		 */
		int64 _node_count;

		/**
		 * The position being searched
		 */
		Handle< Position > _position;

		/**
		 * The principal variation
		 */
		BUFFER( int32, _pv, max_ply, max_ply );

		/**
		 * Number of quiescent nodes visited
		 */
		int64 _qnode_count;

		/**
		 * Number of repetitions encountered
		 */
		int _reps;

		/**
		 * The time to search started
		 */
		int64 _start_time;

		/**
		 * The time to stop searching
		 */
		int64 _stop_time;
	};

	/**
	 * Utility function for searching a list of moves
	 *
	 * @param[in]     phase    The current search phase
	 * @param[in,out] alpha    The search window lower bound
	 * @param[in]     beta     The search window upper bound
	 * @param[in]     depth    The current depth
	 * @param[in]     do_null  Null move enable
	 * @param[in]     best     Best move, if alpha was raised
	 *
	 * @return The search score (relative to us)
	 */
	template <phase_t P>
	int16 Search::search_moves(SearchPhase& phase,
							   int16& alpha, int16 beta,
							   int depth, bool do_null,
							   bool do_zws, int& best)
	{
		int32 move;
		while (phase.next_move<P>(move))
		{
			++_node_count;

			_position->make_move (move);

			int16 score;
			if (do_zws)
			{
				score = -search(depth + 1, -alpha-1, -alpha, do_null);

				if (score > alpha)
					score = -search(depth +1, -beta, -alpha, do_null);
			}
			else
			{
				score = -search(
					depth + 1, -beta, -alpha, do_null);
			}

			_position->unmake_move(move);

			if (beta <= score)
			{
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
}

#endif
