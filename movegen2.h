#ifndef __MOVEGEN__
#define __MOVEGEN__

#include "position2.h"


/**
 **********************************************************************
 *
 * @class MoveGen
 *
 * Generates captures, non-captures, checks, and check evasions, all
 * which are strictly legal
 *
 **********************************************************************
 */
class MoveGen
{

public:

	MoveGen(const DataTables& tables);

	~MoveGen();

	uint32 generate_captures(const Position& pos, int to_move,
							 uint32* captures) const;

	uint32 generate_check_evasions(const Position& pos, int to_move,
								   uint32* evasions) const;
	
	uint32 generate_checks(const Position& pos, int to_move,
						   uint32* checks) const;

	uint32 generate_legal_moves(const Position& pos, int to_move,
								uint32* moves) const;

	uint32 generate_non_captures(const Position& pos, int to_move,
								 uint32* moves) const;

	bool validate_move(const Position& pos, int move,
					   bool in_check) const;

private:

	const DataTables& _tables;
};

/**
 **********************************************************************
 *
 *  Generate captures for a given position. Note that pawn promotions
 *  are included as well. These moves are strictly legal
 *
 * @param[in] pos       A Position
 * @param[in] to_move   The player to generate captures for
 * @param[out] captures The set of captures
 *
 * @return The total number of moves that were generated that capture
 *         another piece or promote
 *
 **********************************************************************
 */
inline uint32 MoveGen::generate_captures(const Position& pos,
	int to_move, uint32* captures) const
{
	const uint64 target = pos._occupied[flip(to_move)];
	const uint64 occupied =
				   pos._occupied[0] | pos._occupied[1];

	uint32 count = 0;

	const uint64 pinned = pos.get_pinned_pieces(to_move);

	/*
	 * Generate pawn captures
	 */
	if (to_move == WHITE)
	{
		uint64 caps = (pos._pawns[WHITE] << 7) & (~FILE_A)
			& pos._occupied[BLACK];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to-7;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_A1H8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 7)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					captures[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				captures[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}

		caps = (pos._pawns[WHITE] << 9) & (~FILE_H)
			& pos._occupied[BLACK];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to-9;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_H1A8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 7)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					captures[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				captures[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}
	}
	else
	{
		uint64 caps = (pos._pawns[BLACK] >> 9) & (~FILE_A)
			& pos._occupied[WHITE];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to+9;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_H1A8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 0)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					captures[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				captures[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}

		caps = (pos._pawns[BLACK] >> 7) & (~FILE_H)
			& pos._occupied[WHITE];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to+7;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_A1H8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 0)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					captures[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				captures[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}
	}

	/*
	 * Generate en passant captures
	 */
	if (pos._ep_info[pos._ply].target != BAD_SQUARE)
	{
		const int from1 =
			pos._ep_info[pos._ply].src[0];
		const int from2 =
			pos._ep_info[pos._ply].src[1];
		const int to =
			pos._ep_info[pos._ply].target;

		if (from1 != BAD_SQUARE)
		{
			bool is_legal = true;

			/*
			 * If the pawn is pinned, make sure the capture is along
			 * the pin direction:
			 */
			if ((_tables.set_mask[from1] & pinned) &&
					_tables.directions[pos._king_sq[to_move]][to] !=
						_tables.directions[from1][to])
			{
				is_legal = false;
			}
			else if (!(_tables.set_mask[from1] & pinned))
			{
				/*
				 * The capturing pawn isn't pinned but we still want
				 * want to prevent against this sort of thing:
				 *
				 * 4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 2
				 *
				 * In this case white still can't capture en passant
				 * because of the rook!
				 */
				const uint64 temp =
							  occupied ^ _tables.set_mask[from1];

				const int vic =
					to_move == WHITE ? (to-8) : (to+8);

				const uint64 rank_attacks =
					pos.attacks_from_rook(vic,temp) & _tables.ranks64[from1];

				const uint64 rooksQueens =
					  pos._rooks[flip(to_move)] | pos._queens[flip(to_move)];

				if ((rank_attacks & pos._kings[to_move]) &&
						(rank_attacks & rooksQueens))
					is_legal = false;
			}

			if (is_legal)
				captures[count++] =
					pack(PAWN, from1, PAWN, INVALID, to);
		}

		if (from2 != BAD_SQUARE)
		{
			bool is_legal = true;

			/*
			 * If the pawn is pinned, make sure the capture is along
			 * the pin direction:
			 */
			if ((_tables.set_mask[from2] & pinned) &&
					_tables.directions[pos._king_sq[to_move]][to] !=
						_tables.directions[from2][to])
			{
				is_legal = false;
			}
			else if (!(_tables.set_mask[from2] & pinned))
			{
				/*
				 * The capturing pawn isn't pinned but we still want
				 * want to prevent against this sort of thing:
				 *
				 * 4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 2
				 *
				 * In this case white still can't capture en passant
				 * because of the rook!
				 */
				const uint64 temp =
								occupied ^ _tables.set_mask[from2];

				const int vic =
					to_move == WHITE ? (to-8) : (to+8);

				const uint64 rank_attacks =
					pos.attacks_from_rook(vic,temp) & _tables.ranks64[from2];

				const uint64 rooksQueens =
					  pos._rooks[flip(to_move)] | pos._queens[flip(to_move)];

				if ((rank_attacks & pos._kings[to_move]) &&
						(rank_attacks & rooksQueens))
					is_legal = false;
			}

			if (is_legal)
				captures[count++] =
					pack(PAWN, from2, PAWN, INVALID, to);
		}
	}

	/*
	 * Generate pawn promotions:
	 */
	if (to_move == WHITE)
	{
		uint64 promotions =
			(pos._pawns[ WHITE ] << 8) & (~occupied) & RANK_8;

		while (promotions)
		{
			const int to = Util::msb64(promotions);
			const int from = to-8;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]]
					!= ALONG_FILE))
			{
				Util::clear_bit64(to, promotions);
				continue;
			}

			for (int p = ROOK; p <= QUEEN; p++)
				captures[count++] =
					pack(INVALID, from, PAWN, p, to);

			Util::clear_bit64(to, promotions);
		}
	}
	else
	{
		uint64 promotions =
			(pos._pawns[ BLACK ] >> 8) & (~occupied) & RANK_1;

		while (promotions)
		{
			const int to = Util::msb64(promotions);
			const int from = to+8;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]]
					!= ALONG_FILE))
			{
				Util::clear_bit64(to, promotions);
				continue;
			}

			for (int p = ROOK; p <= QUEEN; p++)
				captures[count++] =
					pack(INVALID, from, PAWN, p, to);

			Util::clear_bit64(to, promotions);
		}
	}

	/*
	 * Generate knight moves
	 */
	uint64 pieces = pos._knights[to_move] & (~pinned);
	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 _captures = _tables.knight_attacks[from] & target;

		while (_captures)
		{
			const int to = Util::msb64(_captures);

			captures[count++] =
				pack(pos._pieces[to], from, KNIGHT, INVALID, to);
			Util::clear_bit64(to, _captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate bishop moves
	 */
	pieces = pos._bishops[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the bishop is pinned along a file or rank then we cannot
		 * move it, so don't bother generating an attacks_from
		 * bitboard. If it's pinned along an a1-h8 diagonal, clear the
		 * h1-a8 bits of its attacks_from bitboard to ensure we only
		 * keep moves along the direction of the pin (similar idea for
		 * when pinned along an h1-a8 direction):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_FILE:
			case ALONG_RANK:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			default:
				restrictAttacks = _tables.h1a8_64[from];
			}
		}

		uint64 _captures =
			pos.attacks_from_bishop( from, occupied ) & target
				& restrictAttacks;

		while (_captures)
		{
			const int to = Util::msb64(_captures);

			captures[count++] =
				pack(pos._pieces[to],from,BISHOP,INVALID, to);

			Util::clear_bit64(to, _captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate rook moves
	 */
	pieces = pos._rooks[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If this rook is pinned along a diagonal then we cannot move
		 * it, so don't bother generating an attacks_from bitboard.
		 * If pinned along a rank then clear the file bits of its
		 * attacks_from bitboard to ensure we only keep moves along
		 * the direction of the pin (similar reasoning for when pinned
		 * along a file):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
			case ALONG_H1A8:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 _captures =
			pos.attacks_from_rook ( from, occupied ) & target
				& restrictAttacks;

		while (_captures)
		{
			const int to = Util::msb64(_captures);
			captures[count++] =
				pack(pos._pieces[to],from,ROOK, INVALID, to);

			Util::clear_bit64(to, _captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate queen moves
	 */
	pieces = pos._queens[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the queen is pinned, then restrict its motion to along
		 * the direction of the pin:
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			case ALONG_H1A8:
				restrictAttacks = _tables.h1a8_64[from];
				break;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 _captures =
			pos.attacks_from_queen( from, occupied ) & target
				& restrictAttacks;

		while (_captures)
		{
			const int to = Util::msb64(_captures);
			captures[count++] =
				pack(pos._pieces[to],from,QUEEN,INVALID, to);
				
			Util::clear_bit64(to, _captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate king non-castle moves
	 */
	pieces = pos._kings[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 _captures = _tables.king_attacks[ from ] & target;

		while (_captures)
		{
			const int to =
				Util::msb64(_captures);

			if (pos.under_attack(to,flip(to_move)))
			{
				Util::clear_bit64( to, _captures );
				continue;
			}

			captures[count++] =
				pack( pos._pieces[to], from, KING, INVALID, to );

			Util::clear_bit64(to, _captures);
		}

		Util::clear_bit64(from, pieces);
	}

	return count;
}

/**
 **********************************************************************
 *
 * Generate moves that get a king out of check. It's assumed that if
 * this method is called, \a to_move is in check. Note that these
 * moves are strictly legal
 *
 * @param[in] pos     The current position
 * @param[in] to_move Who to generate check evasions for
 * @param[out] moves  The set of moves
 *
 * @return  The total number of moves that were generated that evade
 *          check
 *
 **********************************************************************
 */
inline uint32 MoveGen::generate_check_evasions(const Position& pos,
	int to_move, uint32* moves) const
{
	const uint64 occupied = pos._occupied[0] | pos._occupied[1];

	uint32 count = 0;

	/*
	 * Step 1: Gather all enemy squares attacking our king
	 */
	const uint64 attacks_king =
			pos.attacks_to(pos._king_sq[to_move], flip(to_move));

	/*
	 * Step 2: Generate king moves that get out of check
	 */
	uint64 _moves =
		_tables.king_attacks[pos._king_sq[to_move]]
			& (~pos._occupied[to_move]);

	while (_moves)
	{
		const int to = Util::msb64(_moves);
		Util::clear_bit64(to,_moves);

		const uint64 attack_dir =
			_tables.ray_extend[pos._king_sq[to_move]][to]
				& attacks_king;

		/*
		 * This says if we're in check by a sliding piece, then
		 * do not move along the line of attack unless it is to
		 * capture the checking piece
		 */
		if (((attack_dir & (pos._queens[flip(to_move)] |
				pos._rooks[flip(to_move)])) ||
			 (attack_dir & (pos._queens[flip(to_move)] |
				pos._bishops[flip(to_move)])))
					&& !(_tables.set_mask[to] & attacks_king))
			continue;

		if (!pos.under_attack(to, flip(to_move)))
			moves[count++] =
					pack(pos._pieces[to],pos._king_sq[to_move],
						 KING, INVALID, to);
	}


	/*
	 * Step 3a: If the king is attacked twice, we are done
	 */
	if (attacks_king & (attacks_king-1))
 	   return count;

 	/*
	 * Step 3b: Otherwise, (1) get the square the attacking piece
	 *          is on (the "to" square for capture moves), and
	 *          (2) a bitboard connecting the king square and the
	 *          attacking piece for interposing moves
	 */
 	const int attacker  = Util::msb64(attacks_king);
 	const uint64 target =
 		  	_tables.ray_segment[pos._king_sq[to_move]][attacker];

 	const uint64 pinned =
 		pos.get_pinned_pieces(to_move);

 	/*
	 * Step 4: Generate knight moves
	 */
	uint64 pieces = pos._knights[to_move] & (~pinned);
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
	 	 * Step 4a: Generate knight moves that capture the checking
	 	 *          piece
	 	 */
		uint64
			_moves = _tables.knight_attacks[from] & attacks_king;
		if (_moves)
			moves[count++] = pack(pos._pieces[attacker],
								  from, KNIGHT, INVALID, attacker);

		Util::clear_bit64(from, pieces);

		if (pos._pieces[attacker] == KNIGHT ||
			pos._pieces[attacker] == PAWN )
			continue;

		/*
	 	 * Step 4b: Generate interposing knight moves
	 	 */
		_moves = _tables.knight_attacks[from] & target;
		while (_moves)
		{
			const int to = Util::msb64(_moves);

			moves[count++] =
				  pack(pos._pieces[to], from, KNIGHT, INVALID, to);

			Util::clear_bit64(to, _moves);
		}
	}

	/*
	 * Step 5: Generate rook moves
	 */
	pieces = pos._rooks[to_move] & (~pinned);
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		const uint64 rook_attacks =
				pos.attacks_from_rook(from,occupied);

		/*
	 	 * Step 5a: Generate rook moves that capture the checking
	 	 *          piece
	 	 */
		uint64
			_moves = rook_attacks & attacks_king;
		if (_moves)
			moves[count++] = pack(pos._pieces[attacker],
								  from, ROOK, INVALID, attacker);

		Util::clear_bit64(from, pieces);

		if (pos._pieces[attacker] == KNIGHT ||
			pos._pieces[attacker] == PAWN )
			continue;

		/*
	 	 * Step 5b: Generate interposing rook moves
	 	 */
		_moves = rook_attacks & target;
		while (_moves)
		{
			const int to = Util::msb64(_moves);

			moves[count++] =
				  pack(pos._pieces[to], from, ROOK, INVALID, to);

			Util::clear_bit64(to, _moves);
		}
	}

	/*
	 * Step 6: Generate bishop moves
	 */
	pieces = pos._bishops[to_move] & (~pinned);
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		const uint64 diag_attacks =
				pos.attacks_from_bishop(from,occupied);

		/*
	 	 * Step 6a: Generate bishop moves that capture the checking
	 	 *          piece
	 	 */
		uint64
			_moves = diag_attacks & attacks_king;
		if (_moves)
			moves[count++] = pack(pos._pieces[attacker],
								  from, BISHOP, INVALID, attacker);

		Util::clear_bit64(from, pieces);

		if (pos._pieces[attacker] == KNIGHT ||
			pos._pieces[attacker] == PAWN )
			continue;

		/*
	 	 * Step 6b: Generate interposing bishop moves
	 	 */
		_moves = diag_attacks & target;
		while (_moves)
		{
			const int to = Util::msb64(_moves);

			moves[count++] =
				  pack(pos._pieces[to], from, BISHOP, INVALID, to);

			Util::clear_bit64(to, _moves);
		}
	}

	/*
	 * Step 7: Generate queen moves
	 */
	pieces = pos._queens[to_move] & (~pinned);
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		const uint64 queen_attacks =
				pos.attacks_from_queen(from,occupied);

		/*
	 	 * Step 7a: Generate queen moves that capture the checking
	 	 *          piece
	 	 */
		uint64
			_moves = queen_attacks & attacks_king;
		if (_moves)
			moves[count++] = pack(pos._pieces[attacker],
								  from, QUEEN, INVALID, attacker);

		Util::clear_bit64(from, pieces);

		if (pos._pieces[attacker] == KNIGHT ||
			pos._pieces[attacker] == PAWN )
			continue;

		/*
	 	 * Step 7b: Generate interposing queen moves
	 	 */
		_moves = queen_attacks & target;
		while (_moves)
		{
			const int to = Util::msb64(_moves);

			moves[count++] =
				  pack(pos._pieces[to], from, QUEEN, INVALID, to);

			Util::clear_bit64(to, _moves);
		}
	}

	/*
	 * Step 8: Generate pawn moves
	 */
	pieces = pos._pawns[to_move] & (~pinned);

	/*
	 * Step 8a: Generate pawn moves that capture the checking piece
	 */
    if (to_move == WHITE)
	{
		uint64 caps = (pieces << 7) & (~FILE_A) & attacks_king;
		if (caps)
		{
			const int from = attacker-7;

			if (RANK(attacker) == 7)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] = pack(pos._pieces[attacker],
						from, PAWN, p, attacker);
			}
			else
				moves[count++]     = pack(pos._pieces[attacker],
					from,PAWN,INVALID, attacker);
		}

		caps = (pieces << 9) & (~FILE_H) & attacks_king;
		if (caps)
		{
			const int from = attacker-9;

			if (RANK(attacker) == 7)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] = pack(pos._pieces[attacker],
						from, PAWN, p, attacker);
			}
			else
				moves[count++]     = pack(pos._pieces[attacker],
					from,PAWN,INVALID, attacker);
		}
	}
	else
	{
		uint64 caps = (pieces >> 9) & (~FILE_A) & attacks_king;
		if (caps)
		{
			const int from = attacker+9;
			if (RANK(attacker) == 0)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] = pack(pos._pieces[attacker],
						from, PAWN, p, attacker);
			}
			else
				moves[count++]     = pack(pos._pieces[attacker],
					from,PAWN,INVALID, attacker);

		}

		caps = (pieces >> 7) & (~FILE_H) & attacks_king;
		if (caps)
		{
			const int from = attacker+7;

			if (RANK(attacker) == 0)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] = pack(pos._pieces[attacker],
						from, PAWN, p, attacker);
			}
			else
				moves[count++]     = pack(pos._pieces[attacker],
					from,PAWN,INVALID, attacker);
		}
	}

	/*
	 * Generate en passant captures
	 */
	if (pos._ep_info[pos._ply].target != BAD_SQUARE &&
			(pos._kings[to_move]
				& _tables.pawn_attacks[flip(to_move)][attacker]))
	{
		const int from1 =
			pos._ep_info[pos._ply].src[0];
		const int from2 =
			pos._ep_info[pos._ply].src[1];
		const int to =
			pos._ep_info[pos._ply].target;

		if ((from1 != BAD_SQUARE) &&
				!(_tables.set_mask[from1] & pinned))
		{
			moves[count++] = pack(PAWN, from1,
				PAWN, INVALID, to);
		}

		if ((from2 != BAD_SQUARE) &&
				!(_tables.set_mask[from2] & pinned))
		{
			moves[count++] = pack(PAWN, from2,
				PAWN, INVALID, to);
		}
	}

	/*
	 * If we're in check by a knight or pawn then we're
	 * done (it makes no sense to check for interposing
	 * moves here)
	 */
	if (pos._pieces[attacker] == KNIGHT ||
		pos._pieces[attacker] == PAWN )
		return count;

	/*
	 * Step 8b: Generate interposing pawn moves
	 */
	uint64 advances1, advances2 = 0;

	if (to_move == WHITE)
	{
        advances1 = pos._pawns[WHITE] << 8;
        advances2 =
        	((advances1 & (~occupied)) << 8) & target & RANK_4;
    }
    else
    {
        advances1 = pos._pawns[BLACK] >> 8;
        advances2 =
        	((advances1 & (~occupied)) >> 8) & target & RANK_5;
    }

    advances1 &= target;

    while (advances1)
    {
        const int to = Util::msb64(advances1);
        const int from =
        		(to_move == WHITE) ? (to-8) : (to+8);

        Util::clear_bit64(to, advances1);

        if (pinned & _tables.set_mask[from])
        	continue;

        if (_tables.set_mask[to] & (RANK_8 | RANK_1))
        {
        	for (int p = ROOK; p <= QUEEN; p++)
				moves[count++] =
					pack(INVALID, from, PAWN, p, to);
        }
        else
            moves[count++] =
            	pack(INVALID,from,PAWN, INVALID, to);
    }

    while (advances2)
    {
        const int to = Util::msb64(advances2);
        const int from =
        			(to_move == WHITE) ? (to-16) : (to+16);

        Util::clear_bit64(to, advances2);
        
        if (pinned & _tables.set_mask[from])
            continue;
        
        moves[count++] =
        			pack(INVALID, from, PAWN, INVALID, to);
    }

    return count;
}

/**
 **********************************************************************
 *
 * Generate a set of strictly legal moves that deliver check, but are
 * neither captures nor pawn promotions since those are already
 * generated in \ref generate_captures()
 *
 * @param[in] pos     The current position
 * @param[in] to_move The side to generate checks for
 * @param[out] moves  The set of checks
 *
 * @return The total number of moves that were generated that deliver
 *         check
 *
 **********************************************************************
 */
inline uint32 MoveGen::generate_checks(const Position& pos,
	int to_move, uint32* moves) const
{
	const uint64 occupied =
			pos._occupied[0] | pos._occupied[1];

	const uint64 target = ~occupied;

	uint32 count = 0;

	const uint64 pinned  = pos.get_pinned_pieces (to_move);
	const uint64 xpinned = 
					 pos.get_discover_ready(flip(to_move));

	/*
	 * 1. Generate pawn non-captures and non-promotions
	 *    that uncover check:
	 */
	if (to_move == WHITE)
	{
		/*
		 * 1.1 Generate discovered checks:
		 */
		const uint64 candidates = pos._pawns[WHITE] & xpinned;
		uint64 advances1  =
			(candidates << 8) & (~RANK_8) & (~occupied);
		uint64 advances2 = 
					((advances1 & RANK_3) << 8) & (~occupied);

		while (advances1)
		{
			const int to = Util::msb64(advances1);
			const int from = to-8;

			if (((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_FILE)) ||
				(_tables.directions[from][pos._king_sq[BLACK]] ==
					ALONG_FILE))
			{
				Util::clear_bit64(to, advances1);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, advances1);
		}

		while (advances2)
		{
			const int to = Util::msb64(advances2);
			const int from = to-16;

			if (((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_FILE)) ||
				(_tables.directions[from][pos._king_sq[BLACK]] ==
					ALONG_FILE))
			{
				Util::clear_bit64(to, advances2);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, advances2);
		}

		/*
		 * 1.2 Generate direct checks:
		 */
		const uint64 attack_mask =
				_tables.pawn_attacks[BLACK][pos._king_sq[BLACK]];

		uint64 pawn_adv1 =
			(pos._pawns[WHITE] << 8) & (~RANK_8) & (~occupied);
				
		uint64 pawn_adv2 =
					   ((pawn_adv1 & RANK_3) << 8) & (~occupied);

		pawn_adv1 &= attack_mask;
		pawn_adv2 &= attack_mask;

		while (pawn_adv1)
		{
			const int to = Util::msb64(pawn_adv1);
			const int from = to-8;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_FILE))
			{
				Util::clear_bit64(to, pawn_adv1);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, pawn_adv1);
		}

		while (pawn_adv2)
		{
			const int to = Util::msb64(pawn_adv2);
			const int from = to-16;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_FILE))
			{
				Util::clear_bit64(to, pawn_adv2);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, pawn_adv2);
		}
	}
	else
	{
		/*
		 * 1.1 Generate discovered checks:
		 */
		const uint64 candidates = pos._pawns[BLACK] & xpinned;
		uint64 advances1  =
			(candidates >> 8) & (~RANK_1) & (~occupied);
		uint64 advances2 = 
			  ((advances1 & RANK_6) >> 8) & (~occupied);

		while (advances1)
		{
			const int to = Util::msb64(advances1);
			const int from = to+8;

			if (((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_FILE)) ||
				(_tables.directions[from][pos._king_sq[WHITE]] ==
					ALONG_FILE))
			{
				Util::clear_bit64(to, advances1);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, advances1);
		}

		while (advances2)
		{
			const int to = Util::msb64(advances2);
			const int from = to+16;

			if (((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_FILE)) ||
				(_tables.directions[from][pos._king_sq[WHITE]] ==
					ALONG_FILE))
			{
				Util::clear_bit64(to, advances2);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, advances2);
		}

		/*
		 * 1.2 Generate direct checks:
		 */
		const uint64 attack_mask =
				_tables.pawn_attacks[WHITE][pos._king_sq[WHITE]];

		uint64 pawn_adv1 =
		   (pos._pawns[BLACK] >> 8) & (~RANK_1) & (~occupied);
				
		uint64 pawn_adv2 = 
					((pawn_adv1 & RANK_6) >> 8) & (~occupied);

		pawn_adv1 &= attack_mask;
		pawn_adv2 &= attack_mask;

		while (pawn_adv1)
		{
			const int to = Util::msb64(pawn_adv1);
			const int from = to+8;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_FILE))
			{
				Util::clear_bit64(to, pawn_adv1);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, pawn_adv1);
		}

		while (pawn_adv2)
		{
			const int to = Util::msb64(pawn_adv2);
			const int from = to+16;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_FILE))
			{
				Util::clear_bit64(to, pawn_adv2);
				continue;
			}
			else
				moves[count++] =
					 pack( INVALID, from, PAWN, INVALID, to );

			Util::clear_bit64(to, pawn_adv2);
		}
	}

	/*
	 * 2.1 Generate knight non-captures that deliver discovered
	 *     check
	 */
	uint64 pieces = pos._knights[to_move] & xpinned & (~pinned);
	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 attacks = _tables.knight_attacks[from] & target;

		while (attacks)
		{
			const int to = Util::msb64(attacks);
			moves[count++] =
					pack( INVALID, from, KNIGHT, INVALID, to );
			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * 2.2 Generate knight non-captures that deliver direct
	 *     check
	 */
	pieces =
		pos._knights[to_move] & (~xpinned) & (~pinned);

	const uint64 attacksTo =
			_tables.knight_attacks[pos._king_sq[flip(to_move)]];
	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 attacks =
			_tables.knight_attacks[from] & target & attacksTo;

		while (attacks)
		{
			const int to = Util::msb64(attacks);
			moves[count++] =
					pack ( INVALID, from, KNIGHT, INVALID, to );
			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * 3.1 Generate king non-captures that deliver discovered
	 *     check
	 */
	pieces = pos._kings[ to_move ] & xpinned;
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		uint64 attacks = _tables.king_attacks[from] & target;

		while (attacks)
		{
			const int to = Util::msb64(attacks);

			const int _king_sq  = pos._king_sq[ to_move ];
			const int x_king_sq =
							pos._king_sq[ flip(to_move) ];

			if (pos.under_attack(to, flip(to_move)) ||
				_tables.directions[to][_king_sq] ==
				    _tables.directions[_king_sq][x_king_sq])
			{
				Util::clear_bit64(to, attacks );
					continue;
			}
			moves[count++] =
					pack( INVALID, from, KING, INVALID, to );

			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * 3.2 Generate castle moves that deliver direct check
	 */ 
	if (to_move == WHITE)
	{
		if (pos.can_castle_short(WHITE))
		{
			if (!(occupied & _tables.kingside[WHITE])
				&& !pos.under_attack(F1, BLACK)
				&& !pos.under_attack(G1, BLACK))
			{
				if (pos.attacks_from_rook(F1,
					 occupied ^ pos._kings[WHITE]) & pos._kings[BLACK])
					moves[count++] =
						pack(INVALID, E1, KING, INVALID, G1);
			}
		}

		if (pos.can_castle_long(WHITE))
		{
			if (!(occupied & _tables.queenside[WHITE])
				&& !pos.under_attack(D1, BLACK)
				&& !pos.under_attack(C1, BLACK))
			{
				if (pos.attacks_from_rook(D1,
					 occupied ^ pos._kings[WHITE]) & pos._kings[BLACK])
					moves[count++] =
						pack(INVALID, E1, KING, INVALID, C1);
			}
		}
	}
	else
	{
		if (pos.can_castle_short(BLACK))
		{
			if (!(occupied & _tables.kingside[BLACK])
				&& !pos.under_attack(F8, WHITE)
				&& !pos.under_attack(G8, WHITE))
			{
				if (pos.attacks_from_rook(F8,
					 occupied ^ pos._kings[BLACK]) & pos._kings[WHITE])
					moves[count++] =
						pack(INVALID, E8, KING, INVALID, G8);
			}
		}

		if (pos.can_castle_long(BLACK))
		{
			if (!(occupied & _tables.queenside[BLACK])
				&& !pos.under_attack(D8, WHITE)
				&& !pos.under_attack(C8, WHITE))
			{
				if (pos.attacks_from_rook(D8,
					 occupied ^ pos._kings[BLACK]) & pos._kings[WHITE])
					moves[count++] =
						pack(INVALID, E8, KING, INVALID, C8);
			}
		}
	}

	/*
	 * 4.1 Generate bishop non-captures that deliver discovered
	 *     check
	 */
	pieces = pos._bishops[to_move] & xpinned;
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the bishop is pinned along a file or rank then we cannot
		 * move it, so don't bother generating an attacks_from
		 * bitboard. If it's pinned along an a1-h8 diagonal, clear the
		 * h1-a8 bits of its attacks_from bitboard to ensure we only
		 * keep moves along the direction of the pin (similar idea for
		 * when pinned along an h1-a8 direction):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_FILE:
			case ALONG_RANK:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			default:
				restrictAttacks = _tables.h1a8_64[from];
			}
		}

		uint64 attacks =
			pos.attacks_from_bishop(from, occupied) & target
				& restrictAttacks;

		while (attacks)
		{
			const int to = Util::msb64(attacks);

			moves[count++] =
				pack ( INVALID, from, BISHOP, INVALID, to );
			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * 4.2 Generate bishop non-captures that deliver direct
	 *     check
	 */
	const uint64 diagTarget = 
			pos.attacks_from_bishop(pos._king_sq[flip(to_move)],
				occupied);

	pieces =
		pos._bishops[to_move] & (~xpinned);

	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the bishop is pinned along a file or rank then we cannot
		 * move it, so don't bother generating an attacks_from
		 * bitboard. If it's pinned along an a1-h8 diagonal, clear the
		 * h1-a8 bits of its attacks_from bitboard to ensure we only
		 * keep moves along the direction of the pin (similar idea for
		 * when pinned along an h1-a8 direction):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_FILE:
			case ALONG_RANK:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			default:
				restrictAttacks = _tables.h1a8_64[from];
			}
		}

		uint64 attacks =
			pos.attacks_from_bishop(from, occupied) & target
				& diagTarget & restrictAttacks;

		while (attacks)
		{
			const int to = Util::msb64(attacks);
			
			moves[count++] =
					pack(INVALID,from, BISHOP, INVALID, to);
			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * 5.1 Generate rook non-captures that deliver discovered
	 *     check
	 */
	pieces = pos._rooks[ to_move ] & xpinned;
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If this rook is pinned along a diagonal then we cannot move
		 * it, so don't bother generating an attacks_from bitboard.
		 * If pinned along a rank then clear the file bits of its
		 * attacks_from bitboard to ensure we only keep moves along
		 * the direction of the pin (similar reasoning for when pinned
		 * along a file):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
			case ALONG_H1A8:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 attacks =
			pos.attacks_from_rook(from, occupied) & target
				& restrictAttacks;

		while (attacks)
		{
			const int to = Util::msb64(attacks);

			moves[count++]  = 
				pack ( INVALID, from, ROOK, INVALID, to );
			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * 5.2 Generate rook non-captures that deliver direct
	 *     check
	 */
	const uint64 rookTarget = 
		pos.attacks_from_rook(pos._king_sq[flip(to_move)], occupied);

	pieces = pos._rooks[to_move] & (~xpinned);

	while (pieces)
	{
		const int from =  Util::msb64( pieces );

		/*
		 * If this rook is pinned along a diagonal then we cannot move
		 * it, so don't bother generating an attacks_from bitboard.
		 * If pinned along a rank then clear the file bits of its
		 * attacks_from bitboard to ensure we only keep moves along
		 * the direction of the pin (similar reasoning for when pinned
		 * along a file):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
			case ALONG_H1A8:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 attacks =
			pos.attacks_from_rook(from, occupied) & target
				& rookTarget & restrictAttacks;

		while (attacks)
		{
			const int to = Util::msb64(attacks);

			moves[count++]  =
				pack ( INVALID ,from, ROOK, INVALID, to );
			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * 6. Generate queen non-captures that deliver direct check (queens
	 *    cannot uncover check):
	 */
	const uint64 queenTarget = diagTarget | rookTarget;

	pieces = pos._queens[to_move];

	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the queen is pinned, then restrict its motion to along
		 * the direction of the pin:
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			case ALONG_H1A8:
				restrictAttacks = _tables.h1a8_64[from];
				break;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 attacks =
			pos.attacks_from_queen(from, occupied) & target
				& queenTarget & restrictAttacks;

		while (attacks)
		{
			const int to = Util::msb64(attacks);

			moves[count++] =
				pack ( INVALID, from, QUEEN, INVALID, to );
			Util::clear_bit64(to, attacks);
		}

		Util::clear_bit64(from, pieces);
	}

	return count;
}

/**
 **********************************************************************
 *
 * Generate strictly legal moves from a position
 *
 * Note: Do NOT call this routine if \a to_move is in check. Instead,
 *       use \ref generate_check_evasions()
 *
 * @param[in]  pos     The current position
 * @param[in]  to_move Generate moves for this player
 * @param[out] moves   The set of moves
 *
 * @return The total number of legal moves that were generated for \a
 *         to_move
 *
 **********************************************************************
 */
inline uint32 MoveGen::generate_legal_moves(const Position& pos,
	int to_move, uint32* moves) const
{
	const uint64 target = ~pos._occupied[to_move];
	const uint64 occupied =
			  pos._occupied[0] | pos._occupied[1];

	uint32 count = 0;

	const uint64 pinned =  pos.get_pinned_pieces(to_move);

	/*
	 * Generate pawn captures
	 */
	if (to_move == WHITE)
	{
		uint64 caps = (pos._pawns[WHITE] << 7) & (~FILE_A)
			& pos._occupied[BLACK];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to-7;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_A1H8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 7)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				moves[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}

		caps = (pos._pawns[WHITE] << 9) & (~FILE_H)
			& pos._occupied[BLACK];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to-9;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]] !=
					ALONG_H1A8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 7)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				moves[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}
	}
	else
	{
		uint64 caps = (pos._pawns[BLACK] >> 9) & (~FILE_A)
			& pos._occupied[WHITE];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to+9;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_H1A8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 0)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				moves[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}

		caps = (pos._pawns[BLACK] >> 7) & (~FILE_H)
			& pos._occupied[WHITE];
		while (caps)
		{
			const int to = Util::msb64(caps);
			const int from = to+7;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]] !=
					ALONG_A1H8))
			{
				Util::clear_bit64(to, caps);
				continue;
			}

			if (RANK(to) == 0)
			{
				for (int p = ROOK; p <= QUEEN; p++)
					moves[count++] =
						  pack(pos._pieces[to], from, PAWN, p, to);
			}
			else
				moves[count++] =
					pack(pos._pieces[to], from, PAWN, INVALID, to);

			Util::clear_bit64(to, caps);
		}
	}

	/*
	 * Generate en passant captures
	 */
	if (pos._ep_info[pos._ply].target != BAD_SQUARE)
	{
		const int from1 =
			pos._ep_info[pos._ply].src[0];
		const int from2 =
			pos._ep_info[pos._ply].src[1];
		const int to =
			pos._ep_info[pos._ply].target;

		if (from1 != BAD_SQUARE)
		{
			bool is_legal = true;

			/*
			 * If the pawn is pinned, make sure the capture is along
			 * the pin direction:
			 */
			if ((_tables.set_mask[from1] & pinned) &&
				 _tables.directions[pos._king_sq[to_move]][to] !=
					_tables.directions[from1][to])
			{
				is_legal = false;
			}
			else if (!(_tables.set_mask[from1] & pinned))
			{
				/*
				 * The capturing pawn isn't pinned but we still want
				 * want to prevent against this sort of thing:
				 *
				 * 4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 2
				 *
				 * In this case white still can't capture en passant
				 * because of the rook!
				 */
				const uint64 temp =
							  occupied ^ _tables.set_mask[from1];

				const int vic =
					to_move == WHITE ? (to-8) : (to+8);

				const uint64 rank_attacks =
					pos.attacks_from_rook(vic,temp) & _tables.ranks64[from1];

				const uint64 rooksQueens =
					pos._rooks[flip(to_move)]  |  pos._queens[flip(to_move)];

				if ((rank_attacks & pos._kings[to_move]) &&
						(rank_attacks & rooksQueens))
					is_legal = false;
			}

			if (is_legal)
				moves[count++] =
					pack(PAWN, from1, PAWN, INVALID, to);
		}

		if (from2 != BAD_SQUARE)
		{
			bool is_legal = true;

			/*
			 * If the pawn is pinned, make sure the capture is along
			 * the pin direction:
			 */
			if ((_tables.set_mask[from2] & pinned) &&
				 _tables.directions[pos._king_sq[to_move]][to] !=
					_tables.directions[from2][to])
			{
				is_legal = false;
			}
			else if (!(_tables.set_mask[from2] & pinned))
			{
				/*
				 * The capturing pawn isn't pinned but we still want
				 * want to prevent against this sort of thing:
				 *
				 * 4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 2
				 *
				 * In this case white still can't capture en passant
				 * because of the rook!
				 */
				const uint64 temp =
								occupied ^ _tables.set_mask[from2];

				const int vic =
					to_move == WHITE ? (to-8) : (to+8);

				const uint64 rank_attacks =
					pos.attacks_from_rook(vic,temp) & _tables.ranks64[from2];

				const uint64 rooksQueens =
					pos._rooks[flip(to_move)]  |  pos._queens[flip(to_move)];

				if ((rank_attacks & pos._kings[to_move]) &&
						(rank_attacks & rooksQueens))
					is_legal = false;
			}

			if (is_legal)
				moves[count++] =
					pack(PAWN, from2, PAWN, INVALID, to);
		}
	}

	/*
	 * Generate remaining pawn moves, including promotions:
	 */
	if (to_move == WHITE)
	{
		uint64 advances1 =
			(pos._pawns[WHITE] << 8) & (~occupied);

		uint64 promotions = advances1 & RANK_8;

		// Isolate promotions which are handled separately:
		advances1 ^= promotions;

		while (promotions)
		{
			const int to = Util::msb64(promotions);
			const int from = to-8;

			if ((_tables.set_mask[from] & pinned) &&
					(_tables.directions[from][pos._king_sq[WHITE]]
						!= ALONG_FILE))
			{
				Util::clear_bit64(to, promotions);
				continue;
			}

			for (int p = ROOK; p <= QUEEN; p++)
				moves[count++] = pack(INVALID,from, PAWN, p, to);

			Util::clear_bit64(to, promotions);
		}

		uint64 advances2 =
				  ((advances1 & RANK_3) << 8) & (~occupied);

		while (advances1)
		{
			const int to = Util::msb64(advances1);
			const int from = to-8;

			if ((_tables.set_mask[from] & pinned) &&
					(_tables.directions[from][pos._king_sq[WHITE]]
						!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances1);
				continue;
			}

			moves[count++]= pack(INVALID,from,PAWN, INVALID, to);
			Util::clear_bit64(to, advances1);
		}

		while (advances2)
		{
			const int to = Util::msb64(advances2);
			const int from = to-16;

			if ((_tables.set_mask[from] & pinned) &&
					(_tables.directions[from][pos._king_sq[WHITE]]
						!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances2);
				continue;
			}

			moves[count++] =
				pack(INVALID, from, PAWN, INVALID, to);

			Util::clear_bit64(to, advances2);
		}
	}
	else
	{
		uint64 advances1 =
			(pos._pawns[BLACK] >> 8) & (~occupied);

		uint64 promotions = advances1 & RANK_1;

		// Isolate promotions which are handled separately:
		advances1 ^= promotions;

		while (promotions)
		{
			const int to = Util::msb64(promotions);
			const int from = to+8;

			if ((_tables.set_mask[from] & pinned) &&
					(_tables.directions[from][pos._king_sq[BLACK]]
						!= ALONG_FILE))
			{
				Util::clear_bit64(to, promotions);
				continue;
			}

			for (int p = ROOK; p <= QUEEN; p++)
				moves[count++]= pack(INVALID, from, PAWN, p, to);

			Util::clear_bit64(to, promotions);
		}

		uint64 advances2 =
				 ((advances1 & RANK_6) >> 8) & (~occupied);

		while (advances1)
		{
			const int to   = Util::msb64(advances1);
			const int from = to+8;

			if ((_tables.set_mask[from] & pinned) &&
					(_tables.directions[from][pos._king_sq[BLACK]]
						!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances1);
				continue;
			}

			moves[count++] = pack(INVALID,from,PAWN,INVALID, to);
			Util::clear_bit64(to, advances1);
		}

		while (advances2)
		{
			const int to = Util::msb64(advances2);
			const int from = to+16;

			if ((_tables.set_mask[from] & pinned) &&
					(_tables.directions[from][pos._king_sq[BLACK]]
						!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances2);
				continue;
			}

			moves[count++] =
				pack(INVALID, from, PAWN, INVALID, to);
			Util::clear_bit64(to, advances2);
		}
	}

	/*
	 * Generate knight moves
	 */
	uint64 pieces =
		pos._knights[to_move] & (~pinned);

	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 captures =  _tables.knight_attacks[from] & target;

		while (captures)
		{
			const int to = Util::msb64(captures);

			moves[count++] =
				pack(pos._pieces[to], from, KNIGHT, INVALID, to);
			Util::clear_bit64(to, captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate rook moves
	 */
	pieces = pos._rooks[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If this rook is pinned along a diagonal then we cannot move
		 * it, so don't bother generating an attacks_from bitboard.
		 * If pinned along a rank then clear the file bits of its
		 * attacks_from bitboard to ensure we only keep moves along
		 * the direction of the pin (similar reasoning for when pinned
		 * along a file):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
			case ALONG_H1A8:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 captures =
			pos.attacks_from_rook( from, occupied ) & target
				& restrictAttacks;

		while (captures)
		{
			const int to = Util::msb64(captures);
			moves[count++] =
				pack(pos._pieces[to],from,ROOK,INVALID, to);

			Util::clear_bit64(to, captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate bishop moves
	 */
	pieces = pos._bishops[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the bishop is pinned along a file or rank then we cannot
		 * move it, so don't bother generating an attacks_from
		 * bitboard. If it's pinned along an a1-h8 diagonal, clear the
		 * h1-a8 bits of its attacks_from bitboard to ensure we only
		 * keep moves along the direction of the pin (similar idea for
		 * when pinned along an h1-a8 direction):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_FILE:
			case ALONG_RANK:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			default:
				restrictAttacks = _tables.h1a8_64[from];
			}
		}

		uint64 captures =
			pos.attacks_from_bishop( from, occupied ) & target
				& restrictAttacks;

		while (captures)
		{
			const int to = Util::msb64(captures);
			moves[count++] =
				pack(pos._pieces[to],from,BISHOP,INVALID, to);

			Util::clear_bit64(to, captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate queen moves
	 */
	pieces = pos._queens[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the queen is pinned, then restrict its motion to along
		 * the direction of the pin:
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			case ALONG_H1A8:
				restrictAttacks = _tables.h1a8_64[from];
				break;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 captures =
			pos.attacks_from_queen( from, occupied ) & target
				& restrictAttacks;

		while (captures)
		{
			const int to = Util::msb64(captures);
			moves[count++] =
				pack(pos._pieces[to],from,QUEEN,INVALID,to );
				
			Util::clear_bit64(to, captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate king non-castle moves
	 */
	pieces = pos._kings[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 captures = _tables.king_attacks[from] & target;

		while (captures)
		{
			const int to =
				Util::msb64( captures );

			if (pos.under_attack(to,flip(to_move)))
			{
				Util::clear_bit64 ( to, captures );
				continue;
			}

			moves[count++] =
				pack(pos._pieces[to],from, KING, INVALID, to);

			Util::clear_bit64(to, captures);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate castle moves
	 */ 
	if (to_move == WHITE)
	{
		if (pos.can_castle_short(WHITE))
		{
			if (!(occupied & _tables.kingside[WHITE])
				&& !pos.under_attack(F1, BLACK)
				&& !pos.under_attack(G1, BLACK))
					moves[count++] =
						pack(INVALID, E1, KING, INVALID, G1);
		}

		if (pos.can_castle_long(WHITE))
		{
			if (!(occupied & _tables.queenside[WHITE])
				&& !pos.under_attack(D1, BLACK)
				&& !pos.under_attack(C1, BLACK))
					moves[count++] =
						pack(INVALID, E1, KING, INVALID, C1);
		}
	}
	else
	{
		if (pos.can_castle_short(BLACK))
		{
			if (!(occupied & _tables.kingside[BLACK])
				&& !pos.under_attack(F8, WHITE)
				&& !pos.under_attack(G8, WHITE))
					moves[count++] =
						pack(INVALID, E8, KING, INVALID, G8);
		}

		if (pos.can_castle_long(BLACK))
		{
			if (!(occupied & _tables.queenside[BLACK])
				&& !pos.under_attack(D8, WHITE)
				&& !pos.under_attack(C8, WHITE))
					moves[count++] =
						pack(INVALID, E8, KING, INVALID, C8);
		}
	}

	return count;
}

/**
 **********************************************************************
 *
 *  Generate non-captures for a given position. Note that these moves
 *  are strictly legal
 *
 * @param[in] pos     A Position
 * @param[in] to_move The player to generate moves for
 * @param[out] moves  The set of moves
 *
 * @return The total number of moves generated that do not promote or
 *         capture another piece
 *
 **********************************************************************
 */
inline uint32 MoveGen::generate_non_captures(const Position& pos,
	int to_move, uint32* moves) const
{
	const uint64 occupied =
					 pos._occupied[0] | pos._occupied[1];
	const uint64 target   = ~occupied;
	uint32 count = 0;

	const uint64 pinned = pos.get_pinned_pieces(to_move);

	/*
	 * Generate pawn advances, not including promotions (which
	 * is done in generate_captures())
	 */
	if (to_move == WHITE)
	{
		uint64 advances1 =
			( pos._pawns[ WHITE ] << 8) & (~occupied);

		uint64 promotions = advances1 & RANK_8;

		advances1 ^= promotions;

		uint64 advances2 =
			((advances1 & RANK_3) << 8) & (~occupied);

		while (advances1)
		{
			const int to = Util::msb64(advances1);
			const int from = to-8;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]]
					!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances1);
				continue;
			}

			moves[count++] =
				pack(INVALID, from, PAWN, INVALID, to);
			Util::clear_bit64(to, advances1);
		}

		while (advances2)
		{
			const int to = Util::msb64(advances2);
			const int from = to-16;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[WHITE]]
					!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances2);
				continue;
			}

			moves[count++] =
				pack(INVALID, from, PAWN, INVALID, to);
			Util::clear_bit64(to, advances2);
		}
	}
	else
	{
		uint64 advances1 =
			( pos._pawns[ BLACK ] >> 8) & (~occupied);

		uint64 promotions = advances1 & RANK_1;

		advances1 ^= promotions;

		uint64 advances2 =
			((advances1 & RANK_6) >> 8) & (~occupied);

		while (advances1)
		{
			const int to   = Util::msb64(advances1);
			const int from = to+8;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]]
					!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances1);
				continue;
			}

			moves[count++] =
				pack(INVALID, from, PAWN, INVALID, to);
			Util::clear_bit64(to, advances1);
		}

		while (advances2)
		{
			const int to = Util::msb64(advances2);
			const int from = to+16;

			if ((_tables.set_mask[from] & pinned) &&
				(_tables.directions[from][pos._king_sq[BLACK]]
					!= ALONG_FILE))
			{
				Util::clear_bit64(to, advances2);
				continue;
			}

			moves[count++] =
				pack(INVALID, from, PAWN, INVALID, to);
			Util::clear_bit64(to, advances2);
		}
	}

	/*
	 * Generate knight moves
	 */
	uint64 pieces = pos._knights[to_move] & (~pinned);
	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 _moves = _tables.knight_attacks[ from ] & target;

		while (_moves)
		{
			const int to = Util::msb64(_moves);

			moves[count++] =
				pack(pos._pieces[to],from, KNIGHT, INVALID, to);
			Util::clear_bit64(to, _moves);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate rook moves
	 */
	pieces = pos._rooks[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If this rook is pinned along a diagonal then we cannot move
		 * it, so don't bother generating an attacks_from bitboard.
		 * If pinned along a rank then clear the file bits of its
		 * attacks_from bitboard to ensure we only keep moves along
		 * the direction of the pin (similar reasoning for when pinned
		 * along a file):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
			case ALONG_H1A8:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 _moves =
			pos.attacks_from_rook( from, occupied ) & target
				& restrictAttacks;

		while (_moves)
		{
			const int to = Util::msb64(_moves);
			moves[count++] =
				pack(pos._pieces[to],from,ROOK,INVALID, to);

			Util::clear_bit64(to, _moves);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate bishop moves
	 */
	pieces = pos._bishops[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the bishop is pinned along a file or rank then we cannot
		 * move it, so don't bother generating an attacks_from
		 * bitboard. If it's pinned along an a1-h8 diagonal, clear the
		 * h1-a8 bits of its attacks_from bitboard to ensure we only
		 * keep moves along the direction of the pin (similar idea for
		 * when pinned along an h1-a8 direction):
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_FILE:
			case ALONG_RANK:
				Util::clear_bit64(from, pieces);
				continue;
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			default:
				restrictAttacks = _tables.h1a8_64[from];
			}
		}

		uint64 _moves =
			pos.attacks_from_bishop( from, occupied ) & target
				& restrictAttacks;

		while (_moves)
		{
			const int to = Util::msb64(_moves);
			moves[count++] =
				pack(pos._pieces[to],from,BISHOP, INVALID, to);

			Util::clear_bit64(to, _moves);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate queen moves
	 */
	pieces = pos._queens[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);

		/*
		 * If the queen is pinned, then restrict its motion to along
		 * the direction of the pin:
		 */
		uint64 restrictAttacks = ~0;

		if (_tables.set_mask[from] & pinned)
		{
			switch (_tables.directions[from][pos._king_sq[to_move]])
			{
			case ALONG_A1H8:
				restrictAttacks = _tables.a1h8_64[from];
				break;
			case ALONG_H1A8:
				restrictAttacks = _tables.h1a8_64[from];
				break;
			case ALONG_RANK:
				restrictAttacks = _tables.ranks64[from];
				break;
			default:
				restrictAttacks = _tables.files64[from];
			}
		}

		uint64 _moves =
			pos.attacks_from_queen( from, occupied ) & target
				& restrictAttacks;

		while (_moves)
		{
			const int to = Util::msb64(_moves);
			moves[count++] =
				pack(pos._pieces[to],from,QUEEN,INVALID, to);
				
			Util::clear_bit64(to, _moves);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate king non-castle moves
	 */
	pieces = pos._kings[to_move];
	while (pieces)
	{
		const int from = Util::msb64(pieces);
		uint64 _moves = _tables.king_attacks[ from ] & target;

		while (_moves)
		{
			const int to = Util::msb64(_moves);

			if (pos.under_attack(to, flip(to_move)))
			{
				Util::clear_bit64( to, _moves);
				continue;
			}

			moves[count++] =
				pack(pos._pieces[to],from, KING, INVALID, to);

			Util::clear_bit64(to, _moves);
		}

		Util::clear_bit64(from, pieces);
	}

	/*
	 * Generate castle moves
	 */ 
	if (to_move == WHITE)
	{
		if (pos.can_castle_short(WHITE))
		{
			if (!(occupied & _tables.kingside[WHITE])
				&& !pos.under_attack(F1, BLACK)
				&& !pos.under_attack(G1, BLACK))
					moves[count++] =
						pack(INVALID, E1, KING, INVALID, G1);
		}

		if (pos.can_castle_long(WHITE))
		{
			if (!(occupied & _tables.queenside[WHITE])
				&& !pos.under_attack(D1, BLACK)
				&& !pos.under_attack(C1, BLACK))
					moves[count++] =
						pack(INVALID, E1, KING, INVALID, C1);
		}
	}
	else
	{
		if (pos.can_castle_short(BLACK))
		{
			if (!(occupied & _tables.kingside[BLACK])
				&& !pos.under_attack(F8, WHITE)
				&& !pos.under_attack(G8, WHITE))
					moves[count++] =
						pack(INVALID, E8, KING, INVALID, G8);
		}

		if (pos.can_castle_long(BLACK))
		{
			if (!(occupied & _tables.queenside[BLACK])
				&& !pos.under_attack(D8, WHITE)
				&& !pos.under_attack(C8, WHITE))
					moves[count++] =
						pack(INVALID, E8, KING, INVALID, C8);
		}
	}

	return count;
}

/**
 * Verify that the specified move can be played legally from this
 * position
 *
 * @param [in] pos   The current position
 * @param [in] move  The move to play
 *
 * @return True if the move can be played
 */
inline bool MoveGen::validate_move(const Position& pos, int move,
	bool check) const
{
	const int captured = CAPTURED(move);
	const int from     = FROM(move);
	const int moved    = MOVED(move);
	const int promote  = PROMOTE(move);
	const int to       = TO(move);

	const int to_move = pos._to_move;
	const int ply = pos._ply;

	/*
	 * Verify that (1) the moved piece exists on the origin square,
	 * (2) we occupy the origin square, and (3) we do not
	 * occupy the destination square:
	 */
	if (pos._pieces[from] != moved ||
		!(pos._occupied[to_move] & _tables.set_mask[from]) ||
			(pos._occupied[to_move] & _tables.set_mask[to]) != 0)
		return false;

	if (check)
	{
		/*
		 * Verify we are not trying to castle while in check:
		 */
		if (moved == KING && _abs(from-to) == 2)
			return false;

		const uint64 attacks_king =
			pos.attacks_to(pos._king_sq[to_move], flip(to_move));

		if (attacks_king & (attacks_king-1))
		{
			/*
		 	 * If we're in a double check, and we didn't move
		 	 * the king, this move is illegal:
		 	 */
			if (moved != KING) return false;
		}
		else if (moved != KING)
		{
			/*
			 * If this move neither captures nor blocks the checking
			 * piece, it is illegal:
			 */
			const int attacker = Util::msb64(attacks_king);
			if (to != attacker && !(_tables.set_mask[to]
					& _tables.ray_segment
						[attacker][pos._king_sq[to_move]]))
			{
				return false;
			}
		}
	}

	/*
	 * If this piece is pinned, make sure we're only moving it
	 * along the pin direction
	 */
	direction_t pin_dir = NONE;
	if (moved != KING)
	{
		pin_dir  =  pos.is_pinned( from, to_move );
		if (pin_dir != NONE &&
			pin_dir !=_tables.directions[from][to])
			return false;
	}

	const uint64 occupied =
		pos._occupied[0] | pos._occupied[1];

	bool en_passant = false;
	switch (moved)
	{
	case PAWN:
		if (captured && pos._pieces[to] == INVALID)
		{
			en_passant = true;
			/*
			 * Check if en passant is playable from the position:
			 */
			if (pos._ep_info[ply].target == to &&
				(pos._ep_info[ply].src[ 0 ] != from
					&& pos._ep_info[ply].src[1] != from))
				return false;

			/*
			 * The capturing pawn isn't pinned but we still want
			 * want to prevent against this sort of thing:
			 *
			 * 4k3/8/8/2KPp1r1/8/8/8/8 w - e6 0 2
			 *
			 * In this case white still can't capture en passant
			 * because of the rook!
			 */
			const uint64 temp =
				occupied ^ _tables.set_mask[from ];

			const int vic =
				to_move == WHITE ? (to-8) : (to+8);

			const uint64 rank_attacks =
				pos.attacks_from_rook(vic,temp) & _tables.ranks64[from];

			const uint64 rooksQueens =
				pos._rooks[flip(to_move) ] | pos._queens[flip(to_move)];

			if ((rank_attacks & pos._kings[to_move]) &&
					(rank_attacks & rooksQueens))
				return false;
		}
		else if (_abs(from-to) == 8)
		{
			/*
			 * If this is a pawn advance, make sure the "to" square
			 * is vacant:
			 */
			if (pos._pieces[to] != INVALID)
				return false;
		}
		else if (_abs(from-to) == 16)
		{
			/*
			 * If this is a double pawn advance, make sure both
			 * squares are vacant:
			 */
			const int step1 = to_move == WHITE ? (to-8) : (to+8);
			if (pos._pieces[to] != INVALID ||
					pos._pieces[step1] != INVALID)
				return false;
		}
		break;
	case BISHOP:
	case ROOK:
	case QUEEN:
		/*
		 * If this is a sliding piece, make sure there are no
		 * occupied squares between "from" and "to":
		 */
		if (_tables.ray_segment[from][to] & occupied)
			return false;
		break;
	case KING:
		/*
		 * Note that if this is a castling move, we do not need
		 * to check for a rook on its home square as that's
		 * already taken care of in the pos._castle_rights data
		 */
		if (_abs(from-to) == 2 && !check)
		{
			if (FILE(to) == FILE(G1)
					 && pos.can_castle_short( to_move ))
			{
				if (to_move == WHITE &&
					((occupied &  _tables.kingside[WHITE])
						|| pos.under_attack( F1, BLACK )
						|| pos.under_attack( G1, BLACK )))
				{
					return false;
				}
				else if (to_move == BLACK &&
					((occupied &  _tables.kingside[BLACK])
						|| pos.under_attack( F8, WHITE )
						|| pos.under_attack( G8, WHITE )))
				{
					return false;
				}
			}
			else if (FILE(to) == FILE(C1)
						&& pos.can_castle_long(to_move))
			{
				if (to_move == WHITE &&
					((occupied & _tables.queenside[WHITE])
						|| pos.under_attack( C1, BLACK )
						|| pos.under_attack( D1, BLACK )))
				{
					return false;
				}
				else if (to_move == BLACK &&
					((occupied & _tables.queenside[BLACK])
						|| pos.under_attack( C8, BLACK )
						|| pos.under_attack( D8, BLACK )))
				{
					return false;
				}
			}
		}

		/*
		 * Make sure we aren't trying to move the king
		 * into check:
		 */
		else if (pos.under_attack(to, flip(to_move)) )
			return false;
	}

	/*
	 * If we captured a piece, verify it is on "to" (unless
	 * we played en passant). Note that it isn't worth
	 * checking that the captured piece belongs to the
	 * opponent since we already know we don't have a piece
	 * on the "to" square
	 */
	if (!en_passant && pos._pieces[to] != captured)
		return false;

	return true;
}

#endif
