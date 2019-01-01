#ifndef __SEARCH_PHASE_H__
#define __SEARCH_PHASE_H__

#include "MoveGen4.h"
#include "MoveList.h"
#include "SelectionSort.h"
#include "see.h"

namespace Chess
{
	/**
	 * Enum type describing the current search phase
	 */
	enum class phase_t
	{
		/**
		 * Searching moves that evade check
		 */
		check_evasions,

		/**
		 * Searching winning captures/promotions
		 */
		winning_captures,

		/**
		 * Searching non-captures
		 */
		non_captures,

		/**
		 *  Searching losing captures/promotions
		 */
		losing_captures
	};

	/**
	 * An aggregate that helps coordinate the search
	 */
	struct SearchPhase
	{
		/**
		 * The list of moves that are captures
		 */
		BUFFER(int32, capture_list, max_moves);

		/**
		 * The list of check evasions
		 */
		BUFFER(int32, evasion_list, max_moves);

		/**
		 * The list of non-captures
		 */
		BUFFER(int32, noncapture_list, max_moves);

		/**
		 * Index of the last capture returned
		 */
		int capture_index;

		/**
		 * The sorted list of winning captures
		 */
		SelectionSort winning_captures;

		/**
		 * The sorted list of check evasions
		 */
		SelectionSort evasions;

		/**
		 * The list of non-captures
		 */
		MoveList non_captures;

		/**
		 * The sorted list of losing captures
		 */
		SelectionSort losing_captures;

		/**
		 * Current position (for move scoring)
		 */
		Position* pos;

		/**
		 * Score a move. This is done as follows:
		 *
		 * 1. The score is preliminarily computed as the difference
		 *    in value between the captured and moved pieces
		 * 2. If the move is a promotion, see if we can advance the
		 *    pawn safely. If so, add a bonus equal to the value
		 *    of the piece promoted to
		 *
		 * @param[in] pos  A Position, used to see() captures
		 * @param[in] move The move to score
		 *
		 * @return The score
		 */
		static int score(Position& pos, int32 move)
		{
			auto& tables = DataTables::get();
			int score = tables.exchange[extract_captured(move)][
				extract_moved(move)];

			const piece_t promote = extract_promote(move);
			if (promote != piece_t::empty)
			{
				pos.make_move  (move);

				if (see(pos, pos.get_turn(), extract_to(move)) <= 0)
					score += tables.piece_value[promote];

				pos.unmake_move(move);
			}

			return score;
		}

		/**
		 * Initialize for a particular phase
		 *
		 * @tparam phase The search phase
		 */
		template <phase_t phase>
			void init( Position& _pos );

		/**
		 * Get the next move in a given phase
		 *
		 * @tparam phase The search phase
		 */
		template <phase_t phase>
			bool next_move(int32& move);
	};

	/**
	 * Initialize the evasions phase
	 *
	 * @note Do NOT invoke this phase unless we're in check!
	 *
	 * @param [in] _pos The current position
	 */
	template <>
	void SearchPhase::init<phase_t::check_evasions>(Position& _pos)
	{
		pos = &_pos;
		evasions.init(evasion_list,
			MoveGen::generate_check_evasions(_pos, evasion_list) );
	}

	/**
	 * Initialize the winning captures phase
	 *
	 * @note This should be done prior to initializing for losing
	 *       captures, since here we generate all captures
	 *
	 * @param [in] _pos The current position
	 */
	template <>
	void SearchPhase::init<phase_t::winning_captures>(Position& _pos)
	{
		pos = &_pos;
		capture_index = -1;
		winning_captures.init(capture_list,
			MoveGen::generate_captures(_pos, capture_list ) );
	}

	/**
	 * Initialize the non-captures phase
	 *
	 * @param [in] _pos The current position
	 */
	template <>
	void SearchPhase::init<phase_t::non_captures>(Position& _pos)
	{
		pos = &_pos;
		non_captures.init(noncapture_list,
			MoveGen::generate_noncaptures(_pos, noncapture_list) );
	}

	/**
	 * Initialize the losing captures phase
	 *
	 * @param [in] _pos The current position
	 */
	template <>
	void SearchPhase::init<phase_t::losing_captures>(Position& _pos)
	{
		if (capture_index < 0) capture_index = 0;
		pos = &_pos;
		losing_captures.init(&capture_list[capture_index],
			winning_captures.size - capture_index);
	}

	/**
	 * Get the next check evasion
	 *
	 * @param[out] move The next evasion. If none are left, this
	 *                  is unmodified
	 *
	 * @return True if \a move is valid, or false if the list of
	 *         evasions is exhausted
	 */
	template <>
	bool SearchPhase::next_move<phase_t::check_evasions>(int32& move)
	{
		return evasions.next(move, [](int32 mv1, int32 mv2) {
			return Chess::score(mv1) - Chess::score(mv2);
		});
	}

	/**
	 * Get the next winning capture
	 *
	 * @param[out] move The next capture. If none are left, this
	 *                  is unmodified
	 *
	 * @return True if \a move is valid, or false if the list of
	 *         winning captures is exhausted
	 */
	template <>
	bool SearchPhase::next_move<phase_t::winning_captures>(int32& move)
	{
		const bool valid = winning_captures.next(move,
			[this](int32 mv1, int32 mv2) {
				return score(*this->pos, mv1) - score(*this->pos, mv2 );
		});

		capture_index++;

		return valid && score(*pos, move) > 0;
	}

	/**
	 * Get the next non-capture
	 *
	 * @param [out] move The next non-capture. If none are left,
	 *                   this is unmodified
	 *
	 * @return True if \a move is valid, or false if the list of
	 *         non-captures is exhausted
	 */
	template <>
	bool SearchPhase::next_move<phase_t::non_captures>(int32& move)
	{
		return non_captures.next(move);
	}

	/**
	 * Get the next losing capture
	 *
	 * @param[out] move The next capture. If none are left, this
	 *                  is unmodified
	 *
	 * @return True if \a move is valid, or false if the list of
	 *         losing captures is exhausted
	 */
	template <>
	bool SearchPhase::next_move<phase_t::losing_captures>(int32& move)
	{
		return losing_captures.next(move,
			[this]( int32 mv1, int32 mv2 ) {
				return score(*this->pos, mv1) - score(*this->pos, mv2);
		});
	}
}

#endif
