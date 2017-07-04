#ifndef __MOVEGEN__
#define __MOVEGEN__

#include <iostream>
#include <vector>

#include "chess.h"
#include "position.h"


extern const char* SQUARE_STR[65];


/**
 **********************************************************************
 *
 * @class MoveGen
 *
 * Chess engine move generator
 *
 **********************************************************************
 */
class MoveGen
{

public:

	/**
	 * Constructor
	 *
	 * @param [in] _tables The databases to use (attack boards, LSB
	 *                     lookup, etc.)
	 */
	MoveGen(const DataTables& _tables)
		: tables(_tables)
	{
	}

	/**
	 * Destructor
	 */
	~MoveGen()
	{
	}

	/**
	 * Computes the number of nodes per legal move. This is similar
	 * to perft(), but breaks down the node count by move
	 *
	 * @param[in] pos   The position to run the test on
	 * @param[in] depth The maximum depth to traverse
	 *
	 * @return The number of possible positions up to and including
	 *         \a depth
	 */
	int divide(Position& pos, int depth)
	{
		uint32 moves[MAX_MOVES];

		/*
		 * Generate all possible captures and non-captures
		 */
		uint32* end = 
			generateCaptures(pos, pos.toMove, moves);

		end =
			generateNonCaptures(pos, pos.toMove, end);


		const int nMoves = end - moves;
		int nodes = 0, cumnodes = 0;

		for (register int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);

			if (!pos.inCheck(flip(pos.toMove)))
			{
				if (depth <= 1) nodes = 1;
				else
					nodes = perft( pos, depth-1 );

				const int from = FROM(moves[i]);
				const int to   = TO(moves[i]);

				std::cout << SQUARE_STR[from] << SQUARE_STR[to];

				switch (PROMOTE(moves[i]))
				{
				case KNIGHT:
					std::cout << 'N'; break;
				case ROOK:
					std::cout << 'R'; break;
				case BISHOP:
					std::cout << 'B'; break;
				case QUEEN:
					std::cout << 'Q';
				}
				
				std::cout << ": "
					<< nodes << std::endl;

				cumnodes += nodes;
			}

			pos.unMakeMove(moves[i]);
		}

		return cumnodes;
	}

	/**
	 * Computes the number of nodes per legal move. This is similar
	 * to perft(), but breaks down the node count by move
	 *
	 * @param[in] pos   The position to run the test on
	 * @param[in] depth The maximum depth to traverse
	 *
	 * @return The number of possible positions up to and including
	 *         \a depth
	 */
	int divide2(Position& pos, int depth)
	{
		uint32 moves[MAX_MOVES];

		/*
		 * Generate all possible captures and non-captures
		 */
		uint32* end;

		if (pos.inCheck(pos.toMove))
			end =
			   generateCheckEvasions(pos, pos.toMove, moves);
		else
			end = generateLegalMoves(pos, pos.toMove, moves);

		const int nMoves = end - moves;
		int nodes = 0, cumnodes = 0;

		for (int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);

			nodes = (depth <= 1) ? 1 : perft2(pos,depth - 1);

			const int from = FROM(moves[i]);
			const int to   = TO  (moves[i]);

			std::cout  << SQUARE_STR[from] << SQUARE_STR[to];

			switch (PROMOTE(moves[i]))
			{
			case KNIGHT:
				std::cout << 'N'; break;
			case ROOK:
				std::cout << 'R'; break;
			case BISHOP:
				std::cout << 'B'; break;
			case QUEEN:
				std::cout << 'Q';
			}
			
			std::cout << ": "
				<< nodes << std::endl;

			cumnodes += nodes;

			pos.unMakeMove(moves[i]);
		}

		return cumnodes;
	}

	/**
	 **********************************************************************
	 *
	 * Generate capture moves from the given position, returning a pointer
	 * to the end of the list of moves. This also includes pawn promotions
	 *
	 * @param[in] pos          The current position
	 * @param[in] to_move      Who to generate captures for (doesn't have
	 *                         to be pos.toMove)
	 * @param[in,out] captures The set of captures
	 *
	 * @return Pointer to the index immediately following the last capture
	 *         move
	 *
	 **********************************************************************
	 */
	uint32* generateCaptures(const Position& pos, int to_move,
							 uint32* captures) const
	{
		const uint64 target = pos.occupied[flip(to_move)];
		const uint64 occupied =
						pos.occupied[0] | pos.occupied[1];
		uint32* move = captures;

		/*
		 * Generate pawn captures
		 */
		if (to_move == WHITE)
		{
			uint64 caps = (pos.pawns[WHITE] << 7) & (~FILE_A) & target;
			while (caps)
			{
				int to = getMSB64(caps);

				if (RANK(to) == 7)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], to-7, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], to-7, PAWN, INVALID, to);

				clearBit64(to, caps);
			}

			caps = (pos.pawns[WHITE] << 9) & (~FILE_H) & target;
			while (caps)
			{
				int to = getMSB64(caps);

				if (RANK(to) == 7)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], to-9, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], to-9, PAWN, INVALID, to);

				clearBit64(to, caps);
			}
		}
		else
		{
			uint64 caps = (pos.pawns[BLACK] >> 9) & (~FILE_A) & target;
			while (caps)
			{
				int to = getMSB64(caps);

				if (RANK(to) == 0)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], to+9, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], to+9, PAWN, INVALID, to);

				clearBit64(to, caps);
			}

			caps = (pos.pawns[BLACK] >> 7) & (~FILE_H) & target;
			while (caps)
			{
				int to = getMSB64(caps);

				if (RANK(to) == 0)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], to+7, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], to+7, PAWN, INVALID, to);

				clearBit64(to, caps);
			}
		}

		/*
		 * Generate en passant captures
		 */
		if (pos.epInfo[pos.ply].target != BAD_SQUARE)
		{
			if (pos.epInfo[pos.ply].src[0] != BAD_SQUARE)
				*move++ = pack(PAWN, pos.epInfo[pos.ply].src[0],
								PAWN, INVALID, pos.epInfo[pos.ply].target);

			if (pos.epInfo[pos.ply].src[1] != BAD_SQUARE)
				*move++ = pack(PAWN, pos.epInfo[pos.ply].src[1],
								PAWN, INVALID, pos.epInfo[pos.ply].target);
		}

		/*
		 * Generate pawn promotions
		 */
		if (to_move == WHITE)
		{
			uint64 promotions =
				( pos.pawns[WHITE] << 8 ) & (~occupied) & RANK_8;
			while (promotions)
			{
				int to = getMSB64(promotions);

				for (int p = ROOK; p <= QUEEN; p++)
					*move++ = pack(INVALID, to-8, PAWN, p, to);

				clearBit64(to, promotions);
			}
		}
		else
		{
			uint64 promotions =
				( pos.pawns[BLACK] >> 8 ) & (~occupied) & RANK_1;
			while (promotions)
			{
				int to = getMSB64(promotions);

				for (int p = ROOK; p <= QUEEN; p++)
					*move++ = pack( INVALID, to+8, PAWN, p, to );

				clearBit64(to, promotions);
			}
		}

		/*
		 * Generate knight captures
		 */
		uint64 pieces = pos.knights[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
				uint64 captures = tables.knight_attacks[from] & target;

			while (captures)
			{
				int to = getMSB64(captures);

				*move++ =
					   pack(pos.pieces[to], from, KNIGHT, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate rook captures
		 */
		pieces = pos.rooks[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
			uint64 captures =
						pos.attacksFromRook(from, occupied) & target;

			while (captures)
			{
				int to = getMSB64(captures);

				*move++ =
						pack(pos.pieces[to],from, ROOK, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate bishop captures
		 */
		pieces =
			pos.bishops[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
			uint64 captures =
						pos.attacksFromBishop(from, occupied) & target;

			while (captures)
			{
				int to = getMSB64(captures);

				*move++ =
						pack(pos.pieces[to],from, BISHOP, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate queen captures:
		 */
		pieces =
			pos.queens[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
			uint64 attacksFrom =
						pos.attacksFromQueen(from, occupied) & target;

			uint64 captures = attacksFrom & target;
			while (captures)
			{
				int to = getMSB64(captures);

				*move++ =
						pack(pos.pieces[to],from, QUEEN, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate king captures
		 */
		pieces = pos.kings[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
				uint64 captures = tables.king_attacks[from] & target;

			while (captures)
			{
				int to = getMSB64(captures);

				*move++ =
						pack(pos.pieces[to],from, KING, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		return move;
	}

	/**
	 **********************************************************************
	 *
	 * Generate moves that get a king out of check. It's assumed that if
	 * this method is called, \a to_move is in check
	 *
	 * @param[in] pos        The current position
	 * @param[in] to_move    Who to generate check evasions for
	 * @param[in,out] _moves The set of moves
	 *
	 * @return  A pointer to the index in \a moves immediately following
	 *          the last move
	 *
	 **********************************************************************
	 */
	uint32* generateCheckEvasions(const Position& pos, int to_move,
								  uint32* _moves) const
	{
		const uint64 pinned   = getPinnedPieces(to_move, pos);
		const uint64 occupied =
			pos.occupied[0] | pos.occupied[1];

		uint32* move = _moves;

		/*
		 * Step 1: Gather all enemy squares attacking our king
		 */
		const uint64 attacks_king =
				pos.attacksTo(pos.kingSq[to_move], flip(to_move));

		/*
		 * Step 2: Generate king moves that get out of check
		 */
		uint64 moves =
			tables.king_attacks[pos.kingSq[to_move]]
				& (~pos.occupied[to_move]);

		while (moves)
		{
			const int to = getMSB64(moves);
			clearBit64(to,moves);

			const uint64 attack_dir =
				tables.ray_extend[pos.kingSq[to_move]][to]
					& attacks_king;

			/*
			 * This says if we're in check by a sliding piece, then
			 * do not move along the line of attack unless it is to
			 * capture the checking piece
			 */
			if (((attack_dir & (pos.queens[flip(to_move)] |
					pos.rooks[flip(to_move)])) ||
				 (attack_dir & (pos.queens[flip(to_move)] |
					pos.bishops[flip(to_move)])))
						&& !(tables.set_mask[to] & attacks_king))
				continue;

			if (!pos.underAttack(to, flip(to_move)))
				*move++ = pack( pos.pieces[to], pos.kingSq[to_move],
							KING, INVALID, to );
		}


		/*
		 * Step 3a: If the king is attacked twice, we are done
		 */
		if (attacks_king & (attacks_king-1))
     	   return move;

     	/*
		 * Step 3b: Otherwise, (1) get the square the attacking piece
		 *          is on (the "to" square for capture moves), and
		 *          (2) a bitboard connecting the king square and the
		 *          attacking piece for interposing moves
		 */
     	const int attacker  = getMSB64(attacks_king);
     	const uint64 target =
     		  		tables.ray_segment[pos.kingSq[to_move]][attacker];

     	/*
		 * Step 4: Generate knight moves
		 */
		uint64 pieces = pos.knights[to_move] & (~pinned);
		while (pieces)
		{
			const int from = getMSB64(pieces);

			/*
		 	 * Step 4a: Generate knight moves that capture the checking
		 	 *          piece
		 	 */
			uint64
				moves = tables.knight_attacks[from] & attacks_king;
			if (moves)
				*move++ = pack(pos.pieces[attacker],
									from, KNIGHT, INVALID, attacker);

			clearBit64(from, pieces);

			if (pos.pieces[attacker] == KNIGHT ||
				pos.pieces[attacker] == PAWN )
				continue;

			/*
		 	 * Step 4b: Generate interposing knight moves
		 	 */
			moves = tables.knight_attacks[from] & target;
			while (moves)
			{
				const int to = getMSB64(moves);

				*move++ =
					   pack(pos.pieces[to], from, KNIGHT, INVALID, to);

				clearBit64(to, moves);
			}
		}

		/*
		 * Step 5: Generate rook moves
		 */
		pieces = pos.rooks[to_move] & (~pinned);
		while (pieces)
		{
			const int from = getMSB64(pieces);

			const uint64 rook_attacks =
					pos.attacksFromRook(from,occupied);

			/*
		 	 * Step 5a: Generate rook moves that capture the checking
		 	 *          piece
		 	 */
			uint64
				moves = rook_attacks & attacks_king;
			if (moves)
				*move++ = pack(pos.pieces[attacker],
									from, ROOK, INVALID, attacker);

			clearBit64(from, pieces);

			if (pos.pieces[attacker] == KNIGHT ||
				pos.pieces[attacker] == PAWN )
				continue;

			/*
		 	 * Step 5b: Generate interposing rook moves
		 	 */
			moves = rook_attacks & target;
			while (moves)
			{
				const int to = getMSB64(moves);

				*move++ =
					   pack(pos.pieces[to], from, ROOK, INVALID, to);

				clearBit64(to, moves);
			}
		}

		/*
		 * Step 6: Generate bishop moves
		 */
		pieces = pos.bishops[to_move] & (~pinned);
		while (pieces)
		{
			const int from = getMSB64(pieces);

			const uint64 diag_attacks =
					pos.attacksFromBishop(from,occupied);

			/*
		 	 * Step 6a: Generate bishop moves that capture the checking
		 	 *          piece
		 	 */
			uint64
				moves = diag_attacks & attacks_king;
			if (moves)
				*move++ = pack(pos.pieces[attacker],
									from, BISHOP, INVALID, attacker);

			clearBit64(from, pieces);

			if (pos.pieces[attacker] == KNIGHT ||
				pos.pieces[attacker] == PAWN )
				continue;

			/*
		 	 * Step 6b: Generate interposing bishop moves
		 	 */
			moves = diag_attacks & target;
			while (moves)
			{
				const int to = getMSB64(moves);

				*move++ =
					   pack(pos.pieces[to], from, BISHOP, INVALID, to);

				clearBit64(to, moves);
			}
		}

		/*
		 * Step 7: Generate queen moves
		 */
		pieces = pos.queens[to_move] & (~pinned);
		while (pieces)
		{
			const int from = getMSB64(pieces);

			const uint64 queen_attacks =
					pos.attacksFromQueen(from,occupied);

			/*
		 	 * Step 7a: Generate queen moves that capture the checking
		 	 *          piece
		 	 */
			uint64
				moves = queen_attacks & attacks_king;
			if (moves)
				*move++ = pack(pos.pieces[attacker],
									from, QUEEN, INVALID, attacker);

			clearBit64(from, pieces);

			if (pos.pieces[attacker] == KNIGHT ||
				pos.pieces[attacker] == PAWN )
				continue;

			/*
		 	 * Step 7b: Generate interposing queen moves
		 	 */
			moves = queen_attacks & target;
			while (moves)
			{
				const int to = getMSB64(moves);

				*move++ =
					   pack(pos.pieces[to], from, QUEEN, INVALID, to);

				clearBit64(to, moves);
			}
		}

		/*
		 * Step 8: Generate pawn moves
		 */
		pieces = pos.pawns[to_move] & (~pinned);

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
						*move++ = pack(pos.pieces[attacker],
											from, PAWN, p, attacker);
				}
				else
					*move++ = pack(pos.pieces[attacker],
									  from, PAWN, INVALID, attacker);
			}

			caps = (pieces << 9) & (~FILE_H) & attacks_king;
			if (caps)
			{
				const int from = attacker-9;

				if (RANK(attacker) == 7)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ = pack(pos.pieces[attacker],
											from, PAWN, p, attacker);
				}
				else
					*move++ = pack(pos.pieces[attacker],
									  from, PAWN, INVALID, attacker);
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
						*move++ = pack(pos.pieces[attacker],
											from, PAWN, p, attacker);
				}
				else
					*move++ = pack(pos.pieces[attacker],
									  from, PAWN, INVALID, attacker);

			}

			caps = (pieces >> 7) & (~FILE_H) & attacks_king;
			if (caps)
			{
				const int from = attacker+7;

				if (RANK(attacker) == 0)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ = pack(pos.pieces[attacker],
											from, PAWN, p, attacker);
				}
				else
					*move++ = pack(pos.pieces[attacker],
									  from, PAWN, INVALID, attacker);
			}
		}

		/*
		 * Generate en passant captures
		 */
		if (pos.epInfo[pos.ply].target != BAD_SQUARE &&
				(pos.kings[to_move]
					 & tables.pawn_attacks[flip(to_move)][attacker]))
		{
			const int from1 =
				pos.epInfo[pos.ply].src[0];
			const int from2 =
				pos.epInfo[pos.ply].src[1];
			const int to =
				pos.epInfo[pos.ply].target;

			if ((from1 != BAD_SQUARE) &&
					!(tables.set_mask[from1] & pinned))
			{
				*move++ =
					pack(PAWN, from1, PAWN, INVALID, to);
			}

			if ((from2 != BAD_SQUARE) &&
					!(tables.set_mask[from2] & pinned))
			{
				*move++ =
					pack(PAWN, from2, PAWN, INVALID, to);
			}
		}

		/*
		 * If we're in check by a knight or pawn then we're
		 * done (it makes no sense to check for interposing
		 * moves here)
		 */
		if (pos.pieces[attacker] == KNIGHT ||
			pos.pieces[attacker] == PAWN )
			return move;

		/*
		 * Step 8b: Generate interposing pawn moves
		 */
		uint64 advances1, advances2 = 0;

		if (to_move == WHITE)
		{
	        advances1 = pos.pawns[WHITE] << 8;
	        advances2 =
	        	((advances1 & (~occupied)) << 8) & target & RANK_4;
	    }
	    else
	    {
	        advances1 = pos.pawns[BLACK] >> 8;
	        advances2 =
	        	((advances1 & (~occupied)) >> 8) & target & RANK_5;
	    }

	    advances1 &= target;

	    while (advances1)
	    {
	        const int to = getMSB64(advances1);
	        const int from =
	        			(to_move == WHITE) ? (to-8) : (to+8);

	        clearBit64(to, advances1);

	        if (pinned & tables.set_mask[from])
	        	continue;

	        if (tables.set_mask[to] & (RANK_8 | RANK_1))
	        {
	        	for (int p = ROOK; p <= QUEEN; p++)
					*move++ = pack(INVALID, from, PAWN, p, to);
	        }
	        else
	        {
	            *move++ =
	            		pack(INVALID, from, PAWN, INVALID, to);
	        }
	    }

	    while (advances2)
	    {
	        const int to = getMSB64(advances2);
	        const int from =
	        		(to_move == WHITE) ? (to-16) : (to+16);

	        clearBit64(to, advances2);
	        
	        if (pinned & tables.set_mask[from])
	            continue;
	        
	        *move++ =
	        		pack(INVALID, from, PAWN, INVALID, to);
	    }

	    return move;
	}

	/**
	 **********************************************************************
	 *
	 *  Generate strictly legal moves from a position, returning a pointer
	 *  to the end of the list of moves
	 *
	 *  Note: Do NOT call this routine if \a to_move is in check. Instead, 
	 *        use generateCheckEvasions()
	 *
	 * @param[in] pos       The current position
	 * @param[in] to_move   Who to generate moves for (does not have to be
	 *                      pos.toMove)
	 * @param[in,out] moves The set of moves
	 *
	 * @return  Pointer to the array index immediately following the final
	 *          move in the list
	 *
	 **********************************************************************
	 */
	uint32* generateLegalMoves(const Position& pos, int to_move,
							   uint32* moves) const
	{
		const uint64 target = ~pos.occupied[to_move];
		const uint64 occupied =
				   pos.occupied[0] | pos.occupied[1];
		uint32* move = moves;

		const uint64 pinned = getPinnedPieces(to_move, pos);

		/*
		 * Generate pawn captures
		 */
		if (to_move == WHITE)
		{
			uint64 caps = (pos.pawns[WHITE] << 7) & (~FILE_A)
				& pos.occupied[BLACK];
			while (caps)
			{
				const int to = getMSB64(caps);
				const int from = to-7;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[WHITE]] !=
						ALONG_A1H8))
				{
					clearBit64(to, caps);
					continue;
				}

				if (RANK(to) == 7)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], from, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], from, PAWN, INVALID, to);

				clearBit64(to, caps);
			}

			caps = (pos.pawns[WHITE] << 9) & (~FILE_H)
				& pos.occupied[BLACK];
			while (caps)
			{
				const int to = getMSB64(caps);
				const int from = to-9;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[WHITE]] !=
						ALONG_H1A8))
				{
					clearBit64(to, caps);
					continue;
				}

				if (RANK(to) == 7)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], from, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], from, PAWN, INVALID, to);

				clearBit64(to, caps);
			}
		}
		else
		{
			uint64 caps = (pos.pawns[BLACK] >> 9) & (~FILE_A)
				& pos.occupied[WHITE];
			while (caps)
			{
				const int to = getMSB64(caps);
				const int from = to+9;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[BLACK]] !=
						ALONG_H1A8))
				{
					clearBit64(to, caps);
					continue;
				}

				if (RANK(to) == 0)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], from, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], from, PAWN, INVALID, to);

				clearBit64(to, caps);
			}

			caps = (pos.pawns[BLACK] >> 7) & (~FILE_H)
				& pos.occupied[WHITE];
			while (caps)
			{
				const int to = getMSB64(caps);
				const int from = to+7;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[BLACK]] !=
						ALONG_A1H8))
				{
					clearBit64(to, caps);
					continue;
				}

				if (RANK(to) == 0)
				{
					for (int p = ROOK; p <= QUEEN; p++)
						*move++ =
							  pack(pos.pieces[to], from, PAWN, p, to);
				}
				else
					*move++ =
						pack(pos.pieces[to], from, PAWN, INVALID, to);

				clearBit64(to, caps);
			}
		}

		/*
		 * Generate en passant captures
		 */
		if (pos.epInfo[pos.ply].target != BAD_SQUARE)
		{
			const int from1 =
				pos.epInfo[pos.ply].src[0];
			const int from2 =
				pos.epInfo[pos.ply].src[1];
			const int to =
				pos.epInfo[pos.ply].target;

			if (from1 != BAD_SQUARE)
			{
				bool is_legal = true;

				/*
				 * If the pawn is pinned, make sure the capture is along
				 * the pin direction:
				 */
				if ((tables.set_mask[from1] & pinned)
						&& tables.directions[pos.kingSq[to_move]][to] !=
							tables.directions[from1][to])
				{
					is_legal = false;
				}
				else if (!(tables.set_mask[from1] & pinned))
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
								  occupied ^ tables.set_mask[from1];

					const int vic =
						to_move == WHITE ? (to-8) : (to+8);

					const uint64 rank_attacks =
						pos.attacksFromRook(vic,temp) & tables.ranks64[from1];

					if ((rank_attacks & pos.kings[to_move])
							&& (rank_attacks & pos.rooks[flip(to_move)]))
						is_legal = false;
				}

				if (is_legal)
					*move++ =
						pack(PAWN, from1, PAWN, INVALID, to);
			}

			if (from2 != BAD_SQUARE)
			{
				bool is_legal = true;

				/*
				 * If the pawn is pinned, make sure the capture is along
				 * the pin direction:
				 */
				if ((tables.set_mask[from2] & pinned)
						&& tables.directions[pos.kingSq[to_move]][to] !=
							tables.directions[from2][to])
				{
					is_legal = false;
				}
				else if (!(tables.set_mask[from2] & pinned))
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
									occupied ^ tables.set_mask[from2];

					const int vic =
						to_move == WHITE ? (to-8) : (to+8);

					const uint64 rank_attacks =
						pos.attacksFromRook(vic,temp) & tables.ranks64[from2];

					if ((rank_attacks & pos.kings[to_move])
							&& (rank_attacks & pos.rooks[flip(to_move)]))
						is_legal = false;
				}

				if (is_legal)
					*move++ =
						pack(PAWN, from2, PAWN, INVALID, to);
			}
		}

		/*
		 * Generate remaining pawn moves, including promotions:
		 */
		if (to_move == WHITE)
		{
			uint64 advances1 =
				(pos.pawns[WHITE] << 8) & (~occupied);

			uint64 promotions = advances1 & RANK_8;

			// Isolate promotions which are handled separately:
			advances1 ^= promotions;

			while (promotions)
			{
				const int to = getMSB64(promotions);
				const int from = to-8;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[WHITE]] !=
						ALONG_FILE))
				{
					clearBit64(to, promotions);
					continue;
				}

				for (int p = ROOK; p <= QUEEN; p++)
					*move++  = pack(INVALID, from, PAWN, p, to);

				clearBit64(to, promotions);
			}

			uint64 advances2 =
					  ((advances1 & RANK_3) << 8) & (~occupied);

			while (advances1)
			{
				const int to = getMSB64(advances1);
				const int from = to-8;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[WHITE]] !=
						ALONG_FILE))
				{
					clearBit64(to, advances1);
					continue;
				}

				*move++= pack(INVALID, from, PAWN, INVALID, to);
				clearBit64(to, advances1);
			}

			while (advances2)
			{
				const int to = getMSB64(advances2);
				const int from = to-16;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[WHITE]] !=
						ALONG_FILE))
				{
					clearBit64(to, advances2);
					continue;
				}

				*move++= pack(INVALID, from, PAWN, INVALID, to);
				clearBit64(to, advances2);
			}
		}
		else
		{
			uint64 advances1 =
				(pos.pawns[BLACK] >> 8) & (~occupied);

			uint64 promotions = advances1 & RANK_1;

			// Isolate promotions which are handled separately:
			advances1 ^= promotions;

			while (promotions)
			{
				const int to = getMSB64(promotions);
				const int from = to+8;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[BLACK]] !=
						ALONG_FILE))
				{
					clearBit64(to, promotions);
					continue;
				}

				for (int p = ROOK; p <= QUEEN; p++)
					*move++ = pack(INVALID, from, PAWN, p, to);

				clearBit64(to, promotions);
			}

			uint64 advances2 =
					 ((advances1 & RANK_6) >> 8) & (~occupied);

			while (advances1)
			{
				const int to   = getMSB64(advances1);
				const int from = to+8;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[BLACK]] !=
						ALONG_FILE))
				{
					clearBit64(to, advances1);
					continue;
				}

				*move++ =
						pack(INVALID, from, PAWN, INVALID, to);
				clearBit64(to, advances1);
			}

			while (advances2)
			{
				const int to = getMSB64(advances2);
				const int from = to+16;

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[BLACK]] !=
						ALONG_FILE))
				{
					clearBit64(to, advances2);
					continue;
				}

				*move++ =
						pack(INVALID, from, PAWN, INVALID, to);
				clearBit64(to, advances2);
			}
		}

		/*
		 * Generate knight moves
		 */
		uint64 pieces = pos.knights[to_move] & (~pinned);
		while (pieces)
		{
			const int from = getMSB64(pieces);
				uint64 captures = tables.knight_attacks[from] & target;

			while (captures)
			{
				const int to = getMSB64(captures);

				*move++ =
					   pack(pos.pieces[to], from, KNIGHT, INVALID, to);
				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate rook moves
		 */
		pieces = pos.rooks[to_move];
		while (pieces)
		{
			const int from = getMSB64(pieces);

			uint64 captures =
						pos.attacksFromRook(from, occupied) & target;

			while (captures)
			{
				const int to = getMSB64(captures);

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[to_move]] !=
						tables.directions[from][to]))
				{
					clearBit64(to, captures);
					continue;
				}

				*move++ =
						pack(pos.pieces[to],from, ROOK, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate bishop moves
		 */
		pieces = pos.bishops[to_move];
		while (pieces)
		{
			const int from = getMSB64(pieces);

			uint64 captures =
						pos.attacksFromBishop(from, occupied) & target;

			while (captures)
			{
				const int to = getMSB64(captures);

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[to_move]] !=
						tables.directions[from][to]))
				{
					clearBit64(to, captures);
					continue;
				}

				*move++ =
						pack(pos.pieces[to],from, BISHOP, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate queen moves
		 */
		pieces = pos.queens[to_move];
		while (pieces)
		{
			const int from = getMSB64(pieces);
			uint64 attacksFrom =
						pos.attacksFromQueen(from, occupied) & target;

			uint64 captures = attacksFrom & target;
			while (captures)
			{
				const int to = getMSB64(captures);

				if ((tables.set_mask[from] & pinned) &&
					(tables.directions[from][pos.kingSq[to_move]] !=
						tables.directions[from][to]))
				{
					clearBit64(to, captures);
					continue;
				}

				*move++ =
						pack(pos.pieces[to],from, QUEEN, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate king non-castle moves
		 */
		pieces = pos.kings[to_move];
		while (pieces)
		{
			const int from = getMSB64(pieces);
				uint64 captures = tables.king_attacks[from] & target;

			while (captures)
			{
				const int to = getMSB64(captures);

				if (pos.underAttack(to, flip(to_move)))
				{
					clearBit64(to, captures);
					continue;
				}

				*move++ =
						pack(pos.pieces[to],from, KING, INVALID, to);

				clearBit64(to, captures);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate castle moves
		 */ 
		if (to_move == WHITE)
		{
			if (pos.castleRights[pos.ply][WHITE] & castle_K)
			{
				if (!(occupied & (tables.set_mask[G1] | tables.set_mask[F1]))
					&& !pos.underAttack(F1, BLACK)
					&& !pos.underAttack(G1, BLACK))
						*move++ = pack(INVALID, E1, KING, INVALID, G1);
			}

			if (pos.castleRights[pos.ply][WHITE] & castle_Q)
			{
				if (!(occupied & (tables.set_mask[C1] |
								  tables.set_mask[D1] | tables.set_mask[B1]))
					&& !pos.underAttack(D1, BLACK)
					&& !pos.underAttack(C1, BLACK))
						*move++ = pack(INVALID, E1, KING, INVALID, C1);
			}
		}
		else
		{
			if (pos.castleRights[pos.ply][BLACK] & castle_K)
			{
				if (!(occupied & (tables.set_mask[G8] | tables.set_mask[F8]))
					&& !pos.underAttack(F8, WHITE)
					&& !pos.underAttack(G8, WHITE))
						*move++ = pack(INVALID, E8, KING, INVALID, G8);
			}

			if (pos.castleRights[pos.ply][BLACK] & castle_Q)
			{
				if (!(occupied & (tables.set_mask[C8] |
								  tables.set_mask[D8] | tables.set_mask[B8]))
					&& !pos.underAttack(D8, WHITE)
					&& !pos.underAttack(C8, WHITE))
						*move++ = pack(INVALID, E8, KING, INVALID, C8);
			}
		}

		return move;
	}

	/**
	 **********************************************************************
	 *
	 * Generate non-captures from the given position, returning a pointer
	 * to the end of the list of moves. 
	 *
	 * @param[in] pos          The current position
	 * @param[in] to_move      Who to generate moves for (doesn't have
	 *                         to be pos.toMove)
	 * @param[in,out] captures The set of captures
	 *
	 * @return Pointer to the index immediately following the last capture
	 *         move
	 *
	 **********************************************************************
	 */
	uint32* generateNonCaptures(const Position& pos, int to_move,
								uint32* moves) const
	{
		const uint64 occupied = pos.occupied[0] | pos.occupied[1];
		const uint64 target = ~occupied;

		uint32* move = moves;

		/*
		 * Generate pawn moves (but not promotions)
		 */
		if (to_move == WHITE)
		{
			uint64 advances1 =
				(pos.pawns[WHITE] << 8) & target & (~RANK_8);

			uint64 advances2 =
						((advances1 & RANK_3) << 8) & target;

			while (advances1)
			{
				int to = getMSB64(advances1);
				*move++ =
					pack(INVALID, to- 8, PAWN, INVALID, to);
				clearBit64(to, advances1);
			}

			while (advances2)
			{
				int to = getMSB64(advances2);
				*move++ =
					pack(INVALID, to-16, PAWN, INVALID, to);
				clearBit64(to, advances2);
			}
		}
		else
		{
			uint64 advances1 =
				(pos.pawns[BLACK] >> 8) & target & (~RANK_1);

			uint64 advances2 =
						((advances1 & RANK_6) >> 8) & target;

			while (advances1)
			{
				int to = getMSB64(advances1);
				*move++ =
					pack(INVALID, to+ 8, PAWN, INVALID, to);
				clearBit64(to, advances1);
			}

			while (advances2)
			{
				int to = getMSB64(advances2);
				*move++ =
					pack(INVALID, to+16, PAWN, INVALID, to);
				clearBit64(to, advances2);
			}
		}

		/*
		 * Generate knight non-captures
		 */
		uint64 pieces = pos.knights[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
				uint64 moves = tables.knight_attacks[from] & target;

			while (moves)
			{
				int to = getMSB64(moves);
				*move++ =  pack(INVALID, from, KNIGHT, INVALID, to);
				clearBit64(to, moves);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate rook non-captures
		 */
		pieces = pos.rooks[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
			uint64 moves =
					pos.attacksFromRook(from, occupied) & target;

			while (moves)
			{
				int to = getMSB64(moves);
				*move++ = pack(INVALID, from, ROOK, INVALID, to);
				clearBit64(to, moves);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate bishop non-captures
		 */
		pieces =
			pos.bishops[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
			uint64 moves =
					pos.attacksFromBishop(from, occupied) & target;

			while (moves)
			{
				int to = getMSB64(moves);

				*move++ = pack(INVALID, from, BISHOP, INVALID, to);

				clearBit64(to, moves);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate queen non-captures:
		 */
		pieces =
			pos.queens[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
			uint64 attacksFrom =
				  pos.attacksFromQueen(from, occupied) & target;

			uint64 moves = attacksFrom & target;
			while (moves)
			{
				int to = getMSB64(moves);
				*move++ =
						pack(INVALID, from, QUEEN, INVALID, to);
				clearBit64(to, moves);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate king non-captures
		 */
		pieces = pos.kings[to_move];
		while (pieces)
		{
			int from = getMSB64(pieces);
				uint64 moves = tables.king_attacks[from] & target;

			while (moves)
			{
				int to = getMSB64(moves);
				*move++ =  pack(INVALID, from, KING, INVALID, to);
				clearBit64(to, moves);
			}

			clearBit64(from, pieces);
		}

		/*
		 * Generate castle moves
		 */ 
		if (to_move == WHITE)
		{
			if (pos.castleRights[pos.ply][WHITE] & castle_K)
			{
				if (!(occupied & (tables.set_mask[G1] | tables.set_mask[F1]))
					&& !pos.inCheck(WHITE)
					&& !pos.underAttack(F1, BLACK))
						*move++ = pack(INVALID, E1, KING, INVALID, G1);
			}

			if (pos.castleRights[pos.ply][WHITE] & castle_Q)
			{
				if (!(occupied & (tables.set_mask[C1] |
								  tables.set_mask[D1] | tables.set_mask[B1]))
					&& !pos.inCheck(WHITE)
					&& !pos.underAttack(D1, BLACK))
						*move++ = pack(INVALID, E1, KING, INVALID, C1);
			}
		}
		else
		{
			if (pos.castleRights[pos.ply][BLACK] & castle_K)
			{
				if (!(occupied & (tables.set_mask[G8] | tables.set_mask[F8]))
					&& !pos.inCheck(BLACK)
					&& !pos.underAttack(F8, WHITE))
						*move++ = pack(INVALID, E8, KING, INVALID, G8);
			}

			if (pos.castleRights[pos.ply][BLACK] & castle_Q)
			{
				if (!(occupied & (tables.set_mask[C8] |
								  tables.set_mask[D8] | tables.set_mask[B8]))
					&& !pos.inCheck(BLACK)
					&& !pos.underAttack(D8, WHITE))
						*move++ = pack(INVALID, E8, KING, INVALID, C8);
			}
		}

		return move;
	}

	/**
	 * Determine whether or not the proposed move is legal
	 *
	 * @param[in] pos  The current position
	 * @param[in] move The move to check
	 *
	 * @return True if the proposed move is legal
	 */
	static bool isLegal(const Position& pos, int move)
	{
		MoveGen gen(pos.tables);

		uint32 moves[MAX_MOVES];

		uint32*
		end = gen.generateCaptures(
						pos, pos.toMove, moves);
		end = gen.generateNonCaptures(
						pos, pos.toMove,  end );

		const int nMoves = end - moves;

		for (int i = 0; i < nMoves; i++)
		{
			if (moves[i] == move)
			{
				Position copy(pos);
				if (!copy.makeMove(move) || copy.inCheck(pos.toMove))
					break;
				else
					return true;
			}
		}

		return false;
	}

	/**
	 * Performance test. Walks the move generation tree of strictly
	 * legal moves, counting up the number of resulting positions
	 *
	 * @param[in] pos   The position to run the test on
	 * @param[in] depth The maximum depth to traverse
	 *
	 * @return The number of possible positions up to and including
	 *         \a depth
	 */
	int perft(Position& pos, int depth)
	{
		uint32 moves[MAX_MOVES];

		/*
		 * Generate all possible captures and non-captures
		 */
		uint32* end = 
			generateCaptures(pos, pos.toMove, moves);

		end =
			generateNonCaptures(pos, pos.toMove, end);

		const int nMoves = end - moves;
		int nodes = 0;

		for (register int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);

			if (!pos.inCheck(flip(pos.toMove)))
			{
				if (depth <= 1) nodes += 1;
				else
					nodes += perft(pos, depth-1);
			}

			pos.unMakeMove(moves[i]);
		}

		return nodes;
	}

	/**
	 * Performance test. Walks the move generation tree of strictly
	 * legal moves, counting up the number of resulting positions
	 *
	 * @param[in] pos   The position to run the test on
	 * @param[in] depth The maximum depth to traverse
	 *
	 * @return The number of possible positions up to and including
	 *         \a depth
	 */
	int perft2(Position& pos, int depth)
	{
		uint32 moves[MAX_MOVES];

		/*
		 * Generate all possible captures and non-captures
		 */
		uint32* end;

		if (pos.inCheck(pos.toMove))
			end =
			   generateCheckEvasions(pos, pos.toMove, moves);
		else
			end = generateLegalMoves(pos, pos.toMove, moves);

		const int nMoves = end - moves;

		if (depth <= 1)
			return nMoves;

		int nodes = 0;

		for (register int i = 0; i < nMoves; i++)
		{
			pos.makeMove(moves[i]);

			nodes += perft2(pos, depth-1);

			pos.unMakeMove(moves[i]);
		}

		return nodes;
	}

private:

	const DataTables& tables;

	/**
	 **********************************************************************
	 *
	 * Clear the specified bit in a 64-bit word
	 *
	 * @param [in]     bit  The bit to clear
	 * @param [in,out] word     The 64-bit word
	 *
	 **********************************************************************
	 */
	inline void clearBit64(int bit, uint64& word) const
	{
		word &= tables.clear_mask[bit];
	}

	/**
	 **********************************************************************
	 *
	 * Gets the least significant bit set in a 64-bit word in constant
	 * time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The index of the least significant bit set, or -1 if no
	 *         bits are set
	 *
	 **********************************************************************
	 */
	inline int getLSB64(uint64 qword) const
	{
    	qword &= (-qword);
    
		if (qword < 0x0000000010000ULL)
	    	return tables.lsb[qword];

		if (qword < 0x0000100000000ULL)
			return 16 +
				tables.lsb[qword >> 16];

		if (qword < 0x1000000000000ULL)
			return 32 + 
				tables.lsb[qword >> 32];
        
        return 48 +
        	tables.lsb[qword >> 48];
	}

	/**
	 **********************************************************************
	 *
	 * Gets the most significant bit set in a 64-bit word in constant
	 * time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The index of the most significant bit set, or -1 if no
	 *         bits are set
	 *
	 **********************************************************************
	 */
	inline int getMSB64(uint64 qword) const
	{
		if (qword < 0x0000000010000ULL)
	    	return tables.msb[qword];

		if (qword < 0x0000100000000ULL)
			return 16 +
				tables.msb[qword >> 16];

		if (qword < 0x1000000000000ULL)
			return 32 +
				tables.msb[qword >> 32];
		
		return 48 +
			tables.msb[qword >> 48];
	}

	/**
	 **********************************************************************
	 *
	 * Get a bitboard containing all pieces that are pinned on the king
	 * for the specified side
	 *
	 * @param[in] to_move Get pinned pieces for this side
	 *
	 * @return A bitboard with a bit set for each of the pinned squares
	 *
	 **********************************************************************
	 */
	inline uint64 getPinnedPieces(int to_move, const Position& pos) const
	{
		const uint64 occupied =
			pos.occupied[0] | pos.occupied[1];

		uint64 pinned =
			pos.attacksFromQueen(pos.kingSq[to_move], occupied)
				& pos.occupied[to_move];

		for (uint64 temp = pinned; temp; )
		{
			const int sq = getMSB64(temp);

			switch (tables.directions[sq][pos.kingSq[to_move]])
			{
			case ALONG_RANK:
				if (!(pos.attacksFromRook( sq, occupied)
						& tables.ranks64[sq]
						& (pos.rooks[flip(to_move)] |
							pos.queens[flip(to_move)])))
					clearBit64(sq, pinned);
				break;
			case ALONG_FILE:
				if (!(pos.attacksFromRook( sq, occupied)
						& tables.files64[sq]
						& (pos.rooks[flip(to_move)] |
							pos.queens[flip(to_move)])))
					clearBit64(sq, pinned);
				break;
			case ALONG_A1H8:
				if (!(pos.attacksFromBishop(sq, occupied)
					  & tables.a1h8_64[sq]
					  & (pos.bishops[flip(to_move)] |
					  		 pos.queens[flip(to_move)])))
					clearBit64(sq, pinned);
				break;
			case ALONG_H1A8:
				if (!(pos.attacksFromBishop(sq, occupied)
					  & tables.h1a8_64[sq]
					  & (pos.bishops[flip(to_move)] |
					  		 pos.queens[flip(to_move)])))
					clearBit64(sq, pinned);
			}

			clearBit64(sq, temp);
		}

		return pinned;
	}

	/**
	 **********************************************************************
	 *
	 * Returns the population count (number of bits set) in a 64-bit word
	 * in constant time
	 *
	 * @param [in] qword The 64-bit word
	 *
	 * @return The number of bits set
	 *
	 **********************************************************************
	 */
	inline int popCnt64(uint64 qword) const
	{
		if (qword < 0x10000ULL) return tables.pop[qword];

    	if ( qword  <  0x0000100000000ULL )
    		return (tables.pop[ qword & 0xFFFF] +
					tables.pop[(qword >> 16) & 0xFFFF]);

    	if ( qword  <  0x1000000000000ULL )
    		return (tables.pop[ qword & 0xFFFF] +
					tables.pop[(qword >> 16) & 0xFFFF] +
					tables.pop[(qword >> 32) & 0xFFFF]);

    	return (tables.pop[ qword & 0xFFFF] +
    			tables.pop[(qword >> 16) & 0xFFFF] +
    			tables.pop[(qword >> 32) & 0xFFFF] +
    					tables.pop[(qword >> 48) & 0xFFFF]);
	}
};

#endif