#ifndef __POSITION__
#define __POSITION__

#include <cctype>
#include <cstring>
#include <iostream>
#include <string>

#include "DataTables.h"

/*
 * Bit masks for determining castling rights:
 */
#define castle_K 1
#define castle_Q 2

#define clearSet64(c,s,board)     \
	board |= tables.set_mask[s];  \
	board &=                      \
		   tables.clear_mask[c];  \


/**
 **********************************************************************
 *
 * @class Position
 *
 * Represents a chess position
 *
 **********************************************************************
 */
class Position
{
	friend class PositionTest;
	friend class MoveGen;
	friend class xBoard;
	friend class Evaluator;
	friend class Node;

public:

	/**
	 * Constructor (1). Create the initial position
	 *
	 * @param [in] _tables Databases to use
	 * @param [in] xboard  Flag that indicates we're interfacing with xBoard
	 */
	Position(const DataTables& _tables, bool xboard)
		: is_init(false),
		  tables(_tables)
	{
		if (reset("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
				  xboard))
				is_init = true;
	}

	/**
	 * Construct the position given in Forsyth–Edwards Notation
	 *
	 * @param[in] _tables Databases to use
	 * @param[in] fen     A FEN position
	 * @param[in] xboard  True when interfacing with xBoard
	 */
	Position(const DataTables& _tables, const std::string& fen,
			 bool xboard)
		: is_init(false),
		  tables(_tables)
	{
		if (reset(fen,xboard))
					is_init = true;
	}

	/**
	 * Construct a copy of the given position
	 *
	 * @param [in] other The position to copy
	 */
	Position(const Position& other)
		: tables(other.tables)
	{
		*this = other;
	}

	/**
	 * Destructor
	 */
	~Position()
	{
	}

	/**
	 * Generates the squares attacked by the given piece located at the
	 * given square
	 *
	 * @param[in] square   The square this piece is on
	 * @param[in] piece    What piece to generate attacked squares for
	 * @param[in] to_move  Side to move
	 *
	 * @return A bitboard specifying all squares attacked by this piece
	 *         or ~0 on error
	 */
	inline uint64 attacksFrom(int square,
								piece_t piece, int to_move) const
	{
		uint64 occ = occupied[WHITE] | occupied[BLACK];
		switch (piece)
		{
			case ROOK:
				return attacksFromRook(square, occ);
			case KNIGHT:
				return tables.knight_attacks[square];
			case BISHOP:
				return
					attacksFromBishop( square, occ);
			case PAWN:
				return tables.pawn_attacks[to_move][square];
			case KING:
				return tables.king_attacks[square];
			case QUEEN:
				return attacksFromRook(square, occ) |
					   attacksFromBishop(square, occ);
			default:
				AbortIf(true, ~0,
						"Position::attacksFrom(): Invalid piece");
		}

		return 0;
	}

	/**
	 * Get a bitboard containing 1-bits for each square occupied by \a
	 * to_move that has a piece attacking \a square
	 *
	 * @param[in] square  The square to examine
	 * @param[in] to_move The side whose attackers to the given square
	 *                    we want
	 *
	 * @return A bitboard of attackers for \a to_move
	 */
	inline uint64 attacksTo(int square, int to_move) const
	{
		uint64 out = 0;

		const uint64 occupied =
				this->occupied[0] | this->occupied[1];

		out |= tables.pawn_attacks[flip(to_move)][square]
				& pawns[to_move];
		
		out |= tables.knight_attacks[square]
				& knights[to_move];

		out |= attacksFromRook(square, occupied)
				& (rooks[ to_move ] | queens[to_move]);

		out |= attacksFromBishop(square, occupied)
				& (bishops[to_move] | queens[to_move]);

		out |= tables.king_attacks[square]
				& kings[to_move];

		return out;
	}

	/**
	 * Generates a new hash signature for this position. This should
	 * be called for every reset()
	 */
	void generate_hash()
	{
		/*
		 * Generate pseudo-random numbers used for updating the hash
		 * keys
		 */

		std::srand(101687);

		for (int i = 0; i < 2; i++)
		{
			hash_input.castle_rights[0][i] = Util::rand64();
			hash_input.castle_rights[1][i] = Util::rand64();
		}

		for (int i = 0; i < 8; i++)
			hash_input.en_passant[i]       = Util::rand64();

		for (int i = 0; i < 6; i++)
		{
			for (int j = 0; j < 64; j++)
			{
				hash_input.piece[0][i][j]  = Util::rand64();

				hash_input.piece[1][i][j]  = Util::rand64();
			}
		}

		hash_input.to_move = Util::rand64();

		/*
		 * Compute the hash signature for this position
		 */

		uint64& signature  = save_hash[ply];
		signature = 0;

		if (epInfo[ply].target != BAD_SQUARE)
			signature ^=
				hash_input.en_passant[FILE(epInfo[ply].target)];

		if (toMove)
			signature ^= hash_input.to_move;

		if (castleRights[ply][WHITE] & castle_K)
			signature ^= hash_input.castle_rights[WHITE][0];
		if (castleRights[ply][WHITE] & castle_Q)
			signature ^= hash_input.castle_rights[WHITE][1];
		if (castleRights[ply][BLACK] & castle_K)
			signature ^= hash_input.castle_rights[BLACK][0];
		if (castleRights[ply][BLACK] & castle_Q)
			signature ^= hash_input.castle_rights[BLACK][1];

		for (int i = 0; i < 64; i++)
		{
				const int piece_index =  pieces[i]-1;
			if (piece_index < 0) continue;

			if (occupied[BLACK] & tables.set_mask[i])
			{
				signature ^=
						hash_input.piece[BLACK][piece_index][i];
			}
			else
			{
				signature ^=
						hash_input.piece[WHITE][piece_index][i];
			}
		}
	}

	/**
	 * Get the FEN representation of this position
	 *
	 * @return The FEN position, or an empty string if this position was
	 *         not initialized
	 */
	std::string get_fen() const
	{
		int empty = 0; char empty_s[8];

		std::string fen = "";
			if (!is_init) return (fen);

		for (register int i = 63; i >= 0; i--)
		{
			std::sprintf(empty_s, "%d", empty);
			if (pieces[i] != INVALID)
			{
				bool whitePiece =
					static_cast<bool>(tables.set_mask[i] & occupied[WHITE]);

				if (empty != 0)
				{
					fen += empty_s;  empty = 0;
				}

				switch (pieces[i])
				{
				case PAWN:
					fen += (whitePiece ? "P" : "p");
					break;
				case KNIGHT:
					fen += (whitePiece ? "N" : "n");
					break;
				case BISHOP:
					fen += (whitePiece ? "B" : "b");
					break;
				case ROOK:
					fen += (whitePiece ? "R" : "r");
					break;
				case QUEEN:
					fen += (whitePiece ? "Q" : "q");
					break;
				default:
					fen += (whitePiece ? "K" : "k");
				}
			}
			else
				empty++;

			// Start the next rank:
			if (i % 8 == 0)
			{
				if (empty != 0)
				{
					std::sprintf(empty_s, "%d", empty);
					empty = 0;
					fen += empty_s; 
				}
				if (i != 0)
					fen += "/";
			}
		}

		if (toMove == WHITE)
			fen += " w ";
		else
			fen += " b ";

		if (castleRights[ply][WHITE] & castle_K)
			fen += "K";
		if (castleRights[ply][WHITE] & castle_Q)
			fen += "Q";
		if (castleRights[ply][BLACK] & castle_K)
			fen += "k";
		if (castleRights[ply][BLACK] & castle_Q)
			fen += "q";

		if (castleRights[ply][WHITE] == 0 &&
			castleRights[ply][BLACK] == 0)
			fen += "-";

		fen += " ";

		if (epInfo[ply].target != BAD_SQUARE)
			fen += SQUARE_STR[epInfo[ply].target];
		else
			fen += "-";

		char halfMove_s[8];
		char fullMove_s[8];

		std::sprintf(halfMove_s, "%d", halfMove);
		std::sprintf(fullMove_s, "%d", fullMove);

		std::string space = " ";

		fen +=  space + std::string(halfMove_s) +
			    space + fullMove_s;

		return fen;
	}

	/**
	 * Get the 64-bit Zobrist key associated with this position
	 *
	 * @return The hash key
	 */
	inline uint64 get_hash_key() const
	{
		return save_hash[ply];
	}

	/**
	 * Get the player whose turn it is to move
	 *
	 * @return  WHITE or BLACK, or ~0 on error
	 */
	int32 getTurn() const
	{
		if (!is_init) return ~0;
		return toMove;
	}

	/**
	 * Determine if the given side is in check
	 *
	 * @param[in] to_move WHITE or BLACK
	 *
	 * @return True if \a to_move is in check
	 */
	bool inCheck(int to_move) const
	{
		return underAttack(kingSq[to_move], flip(to_move));
	}

	/**
	 * Play the given move from the current position
	 *
	 * @param[in] move The move data bits
	 *
	 * @return True on success
	 */
	bool makeMove(int move)
	{
		/*
		 * Note: Do NOT call isLegal() from here. The check
		 *       for legality should be done at the
		 *       interface level since we need to make the
		 *       move in order to determine if it is legal,
		 *       i.e. if the player would put himself in
		 *       check. Calling isLegal() here would create
		 *       infinite recursion
		 */
		if (!is_init) return false;

		if (move == 0)
		{
			/*
			 * This is a null move. Switch sides and remove
			 * the en passant target:
			 */
			toMove = flip(toMove);
			_saveEP = epInfo[ply];
			epInfo[ply].clear();
			return true;
		}

		const int captured = CAPTURED(move);
		const int from     = FROM(move);
		const int moved    = MOVED(move);
		const int promote  = PROMOTE(move);
		const int to       = TO(move);

		/*
		 * Initialize the hash signature:
		 */
		uint64& hash = save_hash[ply+1];
		hash = save_hash[ply];

		/*
		 * Before doing anything, carry over the castling
		 * rights to the next ply. Later, when we
		 * unMakeMove(), we'll have a record of what this
		 * was prior to making the move: 
		 */
		castleRights[ply+1][0] =
				castleRights[ply][0];
		castleRights[ply+1][1] = 
				castleRights[ply][1];

		/*
		 * If we had en passant in the previous ply, remove it
		 * from the hash key:
		 */
		if (epInfo[ply].target != BAD_SQUARE)
		{
			hash ^=
			  hash_input.en_passant[FILE(epInfo[ply].target)];
		}

		ply++;

		uint64 src; // En passant origin square(s)

		pieces[from] = INVALID;
		pieces[to]   = static_cast<piece_t>(moved);

		clearSet64(from, to, occupied[toMove]);

		/*
		 * Clear the en passant info as it is no longer
		 * valid:
		 */
		epInfo[ply].clear();

		/*
		 * Update the hash entry to reflect the new location of
		 * the piece moved:
		 */
		hash ^=
			hash_input.piece[toMove][moved-1][from];

		if (promote == INVALID)
			hash ^= hash_input.piece[toMove][moved - 1][ to ];
		else
			hash ^= hash_input.piece[toMove][promote-1][ to ];

		switch (moved)
		{
			case PAWN:
				pawns[toMove] &= tables.clear_mask[from];

				pieces[to] = promote ?
					static_cast<piece_t>(promote) : PAWN;

				switch (promote)
				{
					case KNIGHT:
						knights[toMove] |= tables.set_mask[to];
						break;
					case ROOK:
						rooks[toMove]   |= tables.set_mask[to];
						break;
					case QUEEN:
						queens[toMove]  |= tables.set_mask[to];
						break;
					case BISHOP:
						bishops[toMove] |= tables.set_mask[to];
						break;
					default:
						pawns[toMove]   |= tables.set_mask[to];
				}

				// Set the en passant target square:

				if (_abs(from-to) == 16)
				{
					src = pawns[flip(toMove)] & tables.rankAdjacent[to];
					if (toMove == WHITE)
						epInfo[ply].target = to-8;
					else
						epInfo[ply].target = to+8;

					if (src & (tables.set_mask[to+1]))
						epInfo[ply].src[0] = to+1;
					if (src & (tables.set_mask[to-1]))
						epInfo[ply].src[1] = to-1;

					hash ^=
			  			hash_input.en_passant[FILE(epInfo[ply].target)];
				}

				break;

			case KNIGHT:
				clearSet64(from, to, knights[toMove]);
				break;

			case ROOK:
				clearSet64( from, to, rooks[toMove] );

				if ((castleRights[ply][toMove]) &&
					(tables.back_rank[toMove] &
								tables.set_mask[from]))
				{
					switch (FILE(from))
					{
						/*
						 * Reduce castling rights since we moved
						 * this rook:
						 */
						case 0:
							castleRights[ply][toMove]
								&= castle_Q;

							if (castleRights[ply-1][toMove] & castle_K)
								hash ^=
								   hash_input.castle_rights[toMove][0];
						break;
						case 7:
							castleRights[ply][toMove]
								&= castle_K;

							if (castleRights[ply-1][toMove] & castle_Q)
								hash ^=
								   hash_input.castle_rights[toMove][1];
					}
				}

				break;

			case BISHOP:
				clearSet64(from, to, bishops[toMove]);
				break;

			case QUEEN:
				clearSet64(from, to, queens[toMove]);
				break;

			case KING:
				clearSet64( from, to, kings[toMove] );
				kingSq[toMove] = to;

				/*
				 * Remove castling rights from the hash signature:
				 */
				if (castleRights[ply-1][toMove] & castle_K)
					hash ^=
						hash_input.castle_rights[toMove][0];
				if (castleRights[ply-1][toMove] & castle_Q)
					hash ^=
						hash_input.castle_rights[toMove][1];

				/*
				 * Check if this was a castle move and update the
				 * rook bits accordingly:
				 */
				if (_abs(from-to) == 2)
				{
					if (toMove == WHITE)
					{
						if (to == G1)
						{
							hash ^=
								hash_input.piece[WHITE][ROOK-1][H1];
							hash ^=
								hash_input.piece[WHITE][ROOK-1][F1];

							pieces[H1] = INVALID;
							pieces[F1] = ROOK;

							clearSet64(H1, F1, rooks[WHITE]);
							clearSet64(H1, F1,
											occupied[WHITE]);
						}
						else // Queenside castle
						{
							hash ^=
								hash_input.piece[WHITE][ROOK-1][A1];
							hash ^=
								hash_input.piece[WHITE][ROOK-1][D1];

							pieces[A1] = INVALID;
							pieces[D1] = ROOK;

							clearSet64(A1, D1, rooks[WHITE]);
							clearSet64(A1, D1,
											occupied[WHITE]);
						}
					}
					else
					{
						if (to == G8)
						{
							hash ^=
								hash_input.piece[BLACK][ROOK-1][H8];
							hash ^=
								hash_input.piece[BLACK][ROOK-1][F8];

							pieces[H8] = INVALID;
							pieces[F8] = ROOK;

							clearSet64(H8, F8, rooks[BLACK]);
							clearSet64(H8, F8,
											occupied[BLACK]);
						}
						else // Queenside castle
						{
							hash ^=
								hash_input.piece[BLACK][ROOK-1][A8];
							hash ^=
								hash_input.piece[BLACK][ROOK-1][D8];

							pieces[A8] = INVALID;
							pieces[D8] = ROOK;

							clearSet64(A8, D8, rooks[BLACK]);
							clearSet64(A8, D8,
											occupied[BLACK]);
						}
					}
				}

				/*
				 * Clear all castling rights:
				 */
				castleRights[ply][toMove] = 0;
				break;
		}

		if (captured != INVALID)
		{
			if (captured != PAWN)
				hash ^=
					hash_input.piece[flip(toMove)][captured - 1][to];

			switch (captured)
			{
				case PAWN:
					if (occupied[flip(toMove)] & tables.set_mask[to])
					{
						pawns[flip(toMove)] &=
							tables.clear_mask[to];

						/*
						 * XOR out the captured pawn from the "to" square,
						 * otherwise this was an en passant capture in
						 * which case we need to remove the pawn directly
						 * behind us
						 */
						hash ^=
						  hash_input.piece[flip(toMove)][PAWN-1][to];
					}
					else
					{
						// This was an en passant capture:
						if (toMove == WHITE)
						{
							pieces[to-8] = INVALID;

							occupied[BLACK]
								&= tables.clear_mask[to-8];
							pawns[BLACK]
								&= tables.clear_mask[to-8];

							hash ^=
						  		hash_input.piece[BLACK][PAWN-1][to-8];
						}
						else
						{
							pieces[to+8] = INVALID;

							occupied[WHITE]
								&= tables.clear_mask[to+8];
							pawns[WHITE]
								&= tables.clear_mask[to+8];

							hash ^=
						  		hash_input.piece[WHITE][PAWN-1][to+8];
						}
					}

					material[flip(toMove)] -= PAWN_VALUE;
					break;
				case KNIGHT:
					knights[flip(toMove)] &= tables.clear_mask[to];
					material[flip(toMove)] -= KNIGHT_VALUE;
					break;
				case BISHOP:
					bishops[flip(toMove)] &= tables.clear_mask[to];
					material[flip(toMove)] -= BISHOP_VALUE;
					break;
				case QUEEN:
					queens[flip(toMove)]  &= tables.clear_mask[to];
					material[flip(toMove)] -= QUEEN_VALUE;
					break;
				case ROOK:
					rooks[flip(toMove)]   &= tables.clear_mask[to];

					if ((castleRights[ply][flip(toMove)]) &&
						(tables.back_rank[flip(toMove)] &
										tables.set_mask[to]))
					{
						const int x_side = flip(toMove);

						/*
						 * Remove castling rights for the rook that
						 * was captured:
						 */
						switch (FILE(to))
						{
							case 0:
								castleRights[ply][flip(toMove)] &= castle_Q;


								/*
								 * Update the opponent's castle rights Zobrist
								 * key:
								 */
								if (castleRights[ply-1][x_side] & castle_K)
									hash ^=
										hash_input.castle_rights[x_side][0];
							break;
							case 7:
								castleRights[ply][flip(toMove)] &= castle_K;

								/*
								 * Update the opponent's castle rights Zobrist
								 * key:
								 */
								if (castleRights[ply-1][x_side] & castle_Q)
									hash ^=
										hash_input.castle_rights[x_side][1];
						}
					}

					material[flip(toMove)] -= ROOK_VALUE;
					break;
			}

			// Update the enemy occupancy:
			occupied[flip(toMove)] &=
					tables.clear_mask[to];
		}
		else if (moved != PAWN)
			halfMove++;

		if ( toMove == BLACK )
			fullMove += 1;

		toMove = flip(toMove);

			hash ^= hash_input.to_move;
		return true;
	}

	/**
	 * Dump all members to standard output
	 */
	void print(void) const
	{
		std::cout << "Occupied[WHITE]:\n";
		Util::printBitboard(occupied[WHITE]);

		std::cout << "Occupied[BLACK]:\n";
		Util::printBitboard(occupied[BLACK]);

		std::cout << "Kings[WHITE]:\n";
		Util::printBitboard(kings[WHITE]);

		std::cout << "Kings[BLACK]:\n";
		Util::printBitboard(kings[BLACK]);

		std::cout << "Rooks[WHITE]:\n";
		Util::printBitboard(rooks[WHITE]);

		std::cout << "Rooks[BLACK]:\n";
		Util::printBitboard(rooks[BLACK]);

		std::cout << "Pawns[WHITE]:\n";
		Util::printBitboard(pawns[WHITE]);

		std::cout << "Pawns[BLACK]:\n";
		Util::printBitboard(pawns[BLACK]);

		std::cout << "Bishops[WHITE]:\n";
		Util::printBitboard(bishops[WHITE]);

		std::cout << "Bishops[BLACK]:\n";
		Util::printBitboard(bishops[BLACK]);

		std::cout << "Knights[WHITE]:\n";
		Util::printBitboard(knights[WHITE]);

		std::cout << "Knights[BLACK]:\n";
		Util::printBitboard(knights[BLACK]);

		std::cout << "Queens[WHITE]:\n";
		Util::printBitboard(queens[WHITE]);

		std::cout << "Queens[BLACK]:\n";
		Util::printBitboard(queens[BLACK]);

		std::printf("Castle Rights: 0x%X\n",
			(castleRights[ply][BLACK]<< 2) |
			 castleRights[ply][WHITE]);
		std::printf("To Move:       %s\n",
								toMove ? "white": "black");
		std::printf("EP Target:     %s\n",
						   SQUARE_STR[epInfo[ply].target]);
		std::printf("EP Source[0]:  %s\n",
						   SQUARE_STR[epInfo[ply].src[0]]);
		std::printf("EP Source[1]:  %s\n",
						   SQUARE_STR[epInfo[ply].src[1]]);
		std::printf("White King:    %s\n",
								SQUARE_STR[kingSq[WHITE]]);
		std::printf("Black King:    %s\n",
								SQUARE_STR[kingSq[BLACK]]);
		std::printf("Half Move:     %d\n", halfMove);
		std::printf("Full Move:     %d\n", fullMove);
		std::printf("ply:           %d\n", ply);

		std::printf("Material[WHITE] = %u\n",
			material[WHITE]);
		std::printf("Material[BLACK] = %u\n",
			material[BLACK]);

		printBoard();

		std::cout << std::endl;
	}

	/**
	 * Display the current position
	 */
	void printBoard() const
	{
		int prev_rank = 8;
		uint64 one = 1;

		for (int sq = 63; sq >= -1; sq--)
		{
			if (RANK(sq) != prev_rank)
			{
				std::cout << "\n ---+---+---+---+---+---+---+--- \n";
				if (sq == -1) break;

				prev_rank = RANK(sq);
			}

			if (pieces[sq] != INVALID)
			{
				char piece;
				switch (pieces[sq])
				{
					case PAWN:
						piece = 'P'; break;
					case ROOK:
						piece = 'R'; break;
					case KNIGHT:
						piece = 'N'; break;
					case BISHOP:
						piece = 'B'; break;
					case QUEEN:
						piece = 'Q'; break;
					case KING:
						piece = 'K'; break;
				}

				if (occupied[BLACK] & (one << sq))
					piece = Util::to_lower(piece);

				std::cout << "| " << piece << " ";
			}
			else
				std::cout << "|   ";

			if (sq % 8 == 0) std::cout << "|";
		}

		std::cout << std::endl;
	}

	/**
	 * Reset to the position encoded in Forsyth–Edwards Notation (FEN)
	 *
	 * @param [in] fen    A FEN position
	 * @param [in] xboard Indicates we're interfacing with xBoard
	 *
	 * @return True if the new FEN position was successfully processed
	 */
	bool reset(const std::string& fen, bool xboard)
	{
		Position backup(*this);
		int square = 63;

		// Clear member fields:
		setDefault();

		Util::str_v tokens; Util::split(fen, tokens, '/');

		if (tokens.size() != 8)
		{
			if (!xboard)
				std::cout << "Invalid FEN (wrong number of ranks): "
					<< fen << std::endl;
			*this = backup;
			return false;
		}

		for (size_t i = 0; i < 8; i++)
		{
			for (size_t j = 0; j < tokens[i].size(); j++)
			{
				char c = tokens[i][j];
				if (Util::isPiece(c))
				{
					uint64 mask = Util::getBit<uint64>(square);
					pieces[square] =
						Util::piece2enum(c);

					if (Util::to_lower(c) == c)
						occupied[BLACK] |= mask;
					else
						occupied[WHITE] |= mask;
					square -= 1;

					switch (c)
					{
						case 'p':
							pawns[BLACK]   |= mask; break;
						case 'P':
							pawns[WHITE]   |= mask; break;
						case 'r':
							rooks[BLACK]   |= mask; break;
						case 'R':
							rooks[WHITE]   |= mask; break;
						case 'n':
							knights[BLACK] |= mask; break;
						case 'N':
							knights[WHITE] |= mask; break;
						case 'b':
							bishops[BLACK] |= mask; break;
						case 'B':
							bishops[WHITE] |= mask; break;
						case 'q':
							queens[BLACK]  |= mask; break;
						case 'Q':
							queens[WHITE]  |= mask; break;
						case 'k':
							kings [BLACK]  |= mask;
							kingSq[BLACK]  =
								Util::getLSB<uint64>(mask);
							break;
						case 'K':
							kings [WHITE]  |= mask;
							kingSq[WHITE]  =
								Util::getLSB<uint64>(mask);
							break;
						default:
							*this = backup;
							return false;
					}
				}
				else
				{
					if (std::isdigit(c))
						square -= Util::str_to_int32(std::string(&c,1),10);
					else
					{
						if (!xboard)
							std::cout
								<< "Invalid FEN (unexpected character \""
								<< c << "\"):"
									<< fen << std::endl;
						*this = backup;
						return false;
					}
				}

				if ((square < 0 && i != 7) || square < -1)
				{
					if (!xboard)
						std::cout
							<< "Invalid FEN (more than 64 squares given): "
							<< fen << std::endl;
					*this = backup;
					return false;
				}
				else if (square < 0)
				{
					// Move on to whose turn:
					break;
				}
			}
		}

		Util::str_v posn_info;
		Util::split(tokens.back(), posn_info, ' ');

		halfMove = 0;
		fullMove = 1;

		/*
		 * Reset the ply count, which removes all database-driven history
		 */
		ply = 0;

		switch (posn_info.size())
		{
			default:
				// Ignore anything beyond the 6th token instead of
				// returning an error
			case 6:
				fullMove = Util::str_to_int32(posn_info[5],10);
				if (fullMove == 0)
				{
					if (!xboard)
						std::cout << "Invalid FEN (fullmove number): "
							<< fen << std::endl;
					*this = backup;
					return false;
				}
			case 5:
				halfMove = Util::str_to_int32(posn_info[4],10);
			case 4:
				if (posn_info[3] == "-")
					epInfo[ply].target = BAD_SQUARE;
				else
				{
					for (int i = 0; i < 64; i++)
					{
						if (Util::to_lower(posn_info[3]) == SQUARE_STR[i])
						{
							epInfo[ply].target = i; break;
						}
					}

					if (epInfo[ply].target == BAD_SQUARE)
					{
						if (!xboard)
							std::cout << "Invalid FEN (en passant square): "
								<< fen << std::endl;
						*this = backup;
						return false;
					}
				}
			case 3:
				for (size_t i = 0; i < posn_info[2].size(); i++)
				{
					switch (posn_info[2][i])
					{
						case 'K':
							castleRights[ply][WHITE] |= castle_K;
							break;
						case 'Q':
							castleRights[ply][WHITE] |= castle_Q;
							break;
						case 'k':
							castleRights[ply][BLACK] |= castle_K;
							break;
						case 'q':
							castleRights[ply][BLACK] |= castle_Q;
							break;
						case '-':
							if (posn_info[2].size() == 1)
								continue;
						default:
							if (!xboard)
								std::cout << "Invalid FEN (castling rights): "
									<< fen << std::endl;
							*this = backup;
							return (false);
					}
				}
			case 2:
				if (posn_info[1] != "w" && posn_info[1] != "b")
				{
					if (!xboard)
						std::cout << "Invalid FEN (invalid color): "
							<< fen << std::endl;
					*this = backup;
					return false;
				}
				toMove = posn_info[1] == "w" ? WHITE : BLACK;
				break;
			case 1:
				if (!xboard)
					std::cout << "Invalid FEN (unspecified color): "
						<< fen << std::endl;
				*this = backup;
					return false;
		}

		/*
		 * Set the squares from which we can capture via en passant:
		 */
		uint64 src; int pawnSq;
		if (toMove == WHITE)
		{
			pawnSq = epInfo[ply].target-8;
			src = pawns[WHITE] & tables.rankAdjacent[pawnSq];
		}
		else
		{
			pawnSq = epInfo[ply].target+8;
			src = pawns[BLACK] & tables.rankAdjacent[pawnSq];
		}

		if (src & (tables.set_mask[pawnSq+1]))
			epInfo[ply].src[0] = pawnSq+1;
		if (src & (tables.set_mask[pawnSq-1]))
			epInfo[ply].src[1] = pawnSq-1;

		/*
		 * Validate the new position. If it violates any rules of
		 * chess, reject it
		 */
		if (!validate(fen, xboard))
		{
			*this = backup;
				return(false);
		}

		/*
		 * Compute the material score for both sides. This avoids
		 * having to do so during static eval
		 */
		material[WHITE] =
			Util::bitCount(pawns[WHITE])   * PAWN_VALUE   +
			Util::bitCount(knights[WHITE]) * KNIGHT_VALUE + 
			Util::bitCount(bishops[WHITE]) * BISHOP_VALUE + 
			Util::bitCount(rooks[WHITE])   * ROOK_VALUE   +
			Util::bitCount(queens[WHITE])  * QUEEN_VALUE;

		material[BLACK] =
			Util::bitCount(pawns[BLACK])   * PAWN_VALUE   + 
			Util::bitCount(knights[BLACK]) * KNIGHT_VALUE + 
			Util::bitCount(bishops[BLACK]) * BISHOP_VALUE + 
			Util::bitCount(rooks[BLACK])   * ROOK_VALUE   +
			Util::bitCount(queens[BLACK])  * QUEEN_VALUE;

		/*
		 * Generate a hash signature for this position
		 */
		generate_hash();

		is_init = true;
			return is_init;
	}

	/**
	 * Determine if the given square is being attacked by the specified
	 * side
	 *
	 * @param[in] square   The square to examine
	 * @param[in] to_move  Check if this side is attacking the square
	 *
	 * @return  True if the square is under attack by \a to_move, false
	 *          otherwise
	 */
	bool underAttack(int square, int to_move) const
	{
		if ( tables.pawn_attacks[flip(to_move)][square] & pawns[to_move] )
			return true;

		if (tables.king_attacks  [square] & kings  [to_move])
			return true;

		if (tables.knight_attacks[square] & knights[to_move])
			return true;

		const uint64 rooksQueens    =  rooks[to_move]   | queens[to_move];
		const uint64 bishopsQueens  =  bishops[to_move] | queens[to_move];

		uint64 rook_attackers =
			attacksFrom(square, ROOK  , to_move);

		if (rook_attackers & rooksQueens  )
			return true;

		uint64 diag_attackers = 
			attacksFrom(square, BISHOP, to_move);

		if (diag_attackers & bishopsQueens)
			return true;

		return false;
	}

	/**
	 * Undo the given move from the current position. This is essentially
	 * the inverse of makeMove()
	 *
	 * @param[in] move The move data bits
	 *
	 * @return True on success
	 */
	bool unMakeMove(int move)
	{
		if (!is_init) return false;

		if (move == 0)
		{
			/*
			 * This is a null move. Switch sides and restore
			 * the en passant target:
			 */
			toMove = flip(toMove);
			epInfo[ply] = _saveEP;
			return true;
		}

		const int captured = CAPTURED(move);
		const int from     = FROM(move);
		const int moved    = MOVED(move);
		const int promote  = PROMOTE(move);
		const int to       = TO(move);

		/*
		 * Back up to the previous ply to restore castling
		 * and en passant info:
		 */
		ply--;

		toMove = flip(toMove);

		pieces[from] = static_cast<piece_t>(moved);
		pieces[to]   =
					static_cast<piece_t>(captured);

		clearSet64(to, from, occupied[toMove]);

		switch (moved)
		{
			case PAWN:
				pawns[toMove] |= tables.set_mask[from];

				switch (promote)
				{
					case KNIGHT:
						knights[toMove] &= tables.clear_mask[to];
						break;
					case ROOK:
						rooks[toMove]   &= tables.clear_mask[to];
						break;
					case QUEEN:
						queens[toMove]  &= tables.clear_mask[to];
						break;
					case BISHOP:
						bishops[toMove] &= tables.clear_mask[to];
						break;
					default:
						pawns[toMove]   &= tables.clear_mask[to];
				}

				break;

			case KNIGHT:
				clearSet64(to, from, knights[toMove]);
				break;

			case ROOK:
				clearSet64( to, from, rooks[toMove] );
				break;

			case BISHOP:
				clearSet64(to, from, bishops[toMove]);
				break;

			case QUEEN:
				clearSet64( to, from, queens[toMove]);
				break;

			case KING:
				clearSet64( to, from, kings[toMove] );
				kingSq[toMove] = from;

				/*
				 * Check if this was a castle move and update the
				 * rook bits accordingly:
				 */
				if (_abs(from-to) == 2)
				{
					if (toMove == WHITE)
					{
						if (to == G1)
						{
							pieces[F1] = INVALID;
							pieces[H1] = ROOK;

							clearSet64(F1, H1, rooks[WHITE]);
							clearSet64(F1, H1,
											occupied[WHITE]);
						}
						else // Queenside castle
						{
							pieces[D1] = INVALID;
							pieces[A1] = ROOK;

							clearSet64(D1, A1, rooks[WHITE]);
							clearSet64(D1, A1,
											occupied[WHITE]);
						}
					}
					else
					{
						if (to == G8)
						{
							pieces[F8] = INVALID;
							pieces[H8] = ROOK;

							clearSet64(F8, H8, rooks[BLACK]);
							clearSet64(F8, H8,
											occupied[BLACK]);
						}
						else // Queenside castle
						{
							pieces[D8] = INVALID;
							pieces[A8] = ROOK;

							clearSet64(D8, A8, rooks[BLACK]);
							clearSet64(D8, A8,
											occupied[BLACK]);
						}
					}
				}

				break;
		}

		if (captured != INVALID)
		{
			// Update the enemy occupancy:
			occupied[flip(toMove)] |=
					  tables.set_mask[to];

			switch (captured)
			{
				case PAWN:
					material[flip(toMove)] += PAWN_VALUE;

					/*
					 * This was an en passant capture if the target
					 * square matches epInfo.target:
					 */
					if (to == epInfo[ply].target)
					{
						// This was an en passant capture:

						occupied[flip(toMove)] &=
					  				tables.clear_mask[to];

						if (toMove == WHITE)
						{
							pieces[to-8] = PAWN;

							occupied[BLACK]
								|= tables.set_mask[to-8];
							pawns[BLACK]
								|= tables.set_mask[to-8];
						}
						else
						{
							pieces[to+8] = PAWN;

							occupied[WHITE]
								|= tables.set_mask[to+8];
							pawns[WHITE]
								|= tables.set_mask[to+8];
						}
						pieces[to] = INVALID;
					}
					else
					{
						pawns[flip(toMove)] |=
								tables.set_mask[to];
					}
					break;
				case KNIGHT:
					knights[flip(toMove)] |= tables.set_mask[to];
					material[flip(toMove)] += KNIGHT_VALUE;
					break;
				case QUEEN:
					queens[flip(toMove)]  |= tables.set_mask[to];
					material[flip(toMove)] += QUEEN_VALUE;
					break;
				case ROOK:
					rooks[flip(toMove)]   |= tables.set_mask[to];
					material[flip(toMove)] += ROOK_VALUE;
					break;
				case BISHOP:
					bishops[flip(toMove)] |= tables.set_mask[to];
					material[flip(toMove)] += BISHOP_VALUE;
					break;
			}
		}
		else if (moved != PAWN)
			halfMove--;

		if (toMove == BLACK)
				fullMove -= 1;

		return true;
	}

	/**
	 * Validate the position
	 *
	 * @param[in] fen     The FEN representation of the current
	 *                    position
	 * @param[in] xboard  Is true we're interfacing with xBoard
	 *
	 * @return True if this is a valid Position
	 */
	bool validate(const std::string& fen, bool xboard) const
	{
		// Validate against the following rules:
		//
		// 1. No pawns on the 1st or 8th ranks
		// 2. Only two kings on board
		// 3. Side to move cannot capture a king
		// 4. Castling rights make sense (e.g. king is not on its home
		//    square => cannot castle)
		// 5. En passant target makes sense (e.g. there must be a pawn
		//    to capture available)
		// 6. Maximum 8 pawns per side
		// 7. At most 10 of any piece, per side

		// Rule 1:
		if ((pawns[BLACK] | pawns[WHITE]) & (RANK_1 | RANK_8))
		{
			if (!xboard)
				std::cout << "Invalid FEN (pawn(s) on back rank): "
					<< fen << std::endl;
			return false;
		}

		// Rule 2:
		if ((Util::bitCount(kings[WHITE]) != 1) ||
			(Util::bitCount(kings[BLACK]) != 1))
		{
			if (!xboard)
				std::cout<< "Invalid FEN (wrong number of kings): "
					<< fen << std::endl;
			return false;
		}

		// Rule 3:
		if (inCheck(flip(toMove)))
		{
			if (!xboard)
				std::cout << "Invalid FEN (king in check): "
					<< fen << std::endl;
			return false;
		}

		// Rule 4:
		int32 castle_mask = castle_K | castle_Q;

		if (!(kings[WHITE] & Util::getBit<uint64>(E1)))
		{
			if (castleRights[ply][WHITE] & castle_mask)
			{
				if (!xboard)
					std::cout <<"Invalid FEN (white castling rights): "
						<< fen << std::endl;
				return false;
			}
		}
		else
		{
			if ((castleRights[ply][WHITE] & castle_K)
				 && !(rooks[WHITE] & Util::getBit<uint64>(H1)))
			{
				if (!xboard)
					std::cout << "Invalid FEN (white may not castle "
						"kingside): "
						<< fen << std::endl;
				return false; 
			}

			if ((castleRights[ply][WHITE] & castle_Q)
				 && !(rooks[WHITE] & Util::getBit<uint64>(A1)))
			{
				if (!xboard)
					std::cout << "Invalid FEN (white may not castle "
						"queenside): "
						<< fen << std::endl;
				return false; 
			}
		}

		if (!(kings[BLACK] & Util::getBit<uint64>(E8)))
		{
			if (castleRights[ply][BLACK] & castle_mask)
			{
				if (!xboard)
					std::cout <<"Invalid FEN (black castling rights): "
						<< std::endl;
				return false;
			}
		}
		else
		{
			if ((castleRights[ply][BLACK] & castle_K)
				 && !(rooks[BLACK] & Util::getBit<uint64>(H8)))
			{
				if (!xboard)
					std::cout << "Invalid FEN (black may not castle "
						"kingside): "
						<< fen << std::endl;
				return false; 
			}

			if ((castleRights[ply][BLACK] & castle_Q)
				 && !(rooks[BLACK] & Util::getBit<uint64>(A8)))
			{
				if (!xboard)
					std::cout << "Invalid FEN (black may not castle "
						"queenside): "
						<< fen << std::endl;
				return false; 
			}
		}

		// Rule 5:
		if (epInfo[ply].target != BAD_SQUARE)
		{
			bool bad_ep = false;

			if (toMove == WHITE)
			{
				if (RANK(epInfo[ply].target) != 5 ||
					!(pawns[BLACK] & Util::getBit<uint64>(epInfo[ply].target-8)))
				{
					bad_ep = true;
				}
			}
			else
			{
				if (RANK(epInfo[ply].target) != 2 ||
					!(pawns[WHITE] & Util::getBit<uint64>(epInfo[ply].target+8)))
				{
					bad_ep = true;
				}
			}

			if (bad_ep)
			{
				if (!xboard)
					std::cout << "Invalid FEN (En passant target): "
						<< fen << std::endl;
				return false;
			}
		}

		// Rule 6:
		if (Util::bitCount<uint64>(pawns[WHITE]) > 8 || 
			Util::bitCount<uint64>(pawns[BLACK]) > 8)
		{
			if (!xboard)
				std::cout << "Invalid FEN (Max 8 pawns allowed per side): "
					<< fen << std::endl;
			return false;
		}

		// Rule 7:
		if (Util::bitCount<uint64>(knights[WHITE]) > 10 ||
			Util::bitCount<uint64>(knights[BLACK]) > 10)
		{
			if (!xboard)
				std::cout << "Invalid FEN (Max 10 knights allowed per side): "
					<< fen << std::endl;
			return false;
		}

		if (Util::bitCount<uint64>(rooks[WHITE]) > 10 ||
			Util::bitCount<uint64>(rooks[BLACK]) > 10)
		{
			if (!xboard)
				std::cout << "Invalid FEN (Max 10 rooks allowed per side): "
					<< fen << std::endl;
			return false;
		}

		if (Util::bitCount<uint64>(queens[WHITE]) > 10 ||
			Util::bitCount<uint64>(queens[BLACK]) > 10)
		{
			if (!xboard)
				std::cout << "Invalid FEN (Max 10 queens allowed per side): "
					<< fen << std::endl;
			return false;
		}

		if (Util::bitCount<uint64>(bishops[WHITE]) > 10 ||
			Util::bitCount<uint64>(bishops[BLACK]) > 10)
		{
			if (!xboard)
				std::cout << "Invalid FEN (Max 10 bishops allowed per side): "
					<< fen << std::endl;
			return false;
		}
		
		return true;
	}

	/**
	 * Assignment operator
	 *
	 * @param[in] rhs The Position to assign this to
	 *
	 * @return *this
	 */
	Position& operator=(const Position& rhs)
	{
		memcpy(this, &rhs, sizeof(Position));
		return *this;
	}

	/**
	 * Compare this Position against another, byte-wise (except for
	 * hash data)
	 *
	 * @param[in] rhs The Position to compare against
	 *
	 * @return True if both are equal, false otherwise
	 */
	bool operator==(const Position& rhs)
	{
		bool temp = 

			bishops[0]           == rhs.bishops[0]           &&
			bishops[1]           == rhs.bishops[1]           &&
			castleRights[ply][0] == rhs.castleRights[ply][0] && 
			castleRights[ply][1] == rhs.castleRights[ply][1] && 
			epInfo[ply].target   == rhs.epInfo[ply].target   && 
			epInfo[ply].src[0]   == rhs.epInfo[ply].src[0]   &&
			epInfo[ply].src[1]   == rhs.epInfo[ply].src[1]   &&
			fullMove             == rhs.fullMove             &&
			halfMove             == rhs.halfMove             &&
			is_init              == rhs.is_init              && 
			kings[0]             == rhs.kings[0]             &&
			kings[1]             == rhs.kings[1]             &&
			kingSq[0]            == rhs.kingSq[0]            &&
			kingSq[1]            == rhs.kingSq[1]            &&
			knights[0]           == rhs.knights[0]           &&
			knights[1]           == rhs.knights[1]           &&
			material[0]          == rhs.material[0]          &&
			material[1]          == rhs.material[1]          &&
			occupied[0]          == rhs.occupied[0]          &&
			occupied[1]          == rhs.occupied[1]          &&
			pawns[0]             == rhs.pawns[0]             &&
			pawns[1]             == rhs.pawns[1]             &&
			ply                  == rhs.ply                  &&
			queens[0]            == rhs.queens[0]            &&
			queens[1]            == rhs.queens[1]            &&
			rooks[0]             == rhs.rooks[0]             &&
			rooks[1]             == rhs.rooks[1]             &&
			toMove               == rhs.toMove;

		if (temp)
		{
			for (int i = 0; i < 64; i++)
			{
				if (pieces[i] != rhs.pieces[i]) return false;
			}

			/*
			 * Assume the hash signatures match if everything
			 * else matches
			 */
		}

		return temp;
	}

private:

	/**
	 * Generates the squares attacked by a bishop located at the given
	 * square
	 *
	 * @param[in] square   The bishop's location
	 * @param[in] occupied Occupied squares bitboard
	 *
	 * @return A bitboard specifying all squares attacked by this piece
	 */
	inline uint64 attacksFromBishop( int square, uint64 occupied ) const
	{
		return tables.bishop_attacks[tables.bishop_offsets[square] +
					(((occupied & tables.bishop_attacks_mask[square])
						* diag_magics[square]) >> tables.bishop_db_shifts[square])];
	}

	/**
	 * Generates the squares attacked by a queen located at the given
	 * square
	 *
	 * @param[in] square   The queen's location
	 * @param[in] occupied Occupied squares bitboard
	 *
	 * @return  A bitboard showing all squares attacked by this piece
	 */
	inline uint64 attacksFromQueen(int square, uint64 occupied) const
	{
		return attacksFromRook(square, occupied)
				| attacksFromBishop(square, occupied);
	}

	/**
	 * Generates the squares attacked by a rook located at the given
	 * square
	 *
	 * @param[in] square   The rook's location
	 * @param[in] occupied Occupied squares bitboard
	 *
	 * @return A bitboard showing all squares attacked by this piece
	 */
	inline uint64 attacksFromRook(int square, uint64 occupied) const
	{
		return tables.rook_attacks[tables.rook_offsets[square] +
					(((occupied & tables.rook_attacks_mask[square])
						* rook_magics[square]) >> tables.rook_db_shifts[square])];
	}

	/**
	 * Get the mobility of a bishop on \a square. This is hashed to
	 * avoid computing it on the fly
	 *
	 * @param[in] square   The bishop's location
	 * @param[in] occupied Occupied squares bitboard
	 *
	 * @return A popcnt() of attacksFromBishop()
	 */
	inline int getBishopMobility(int square, uint64 occupied) const
	{
		return tables.bishop_mobility[tables.bishop_offsets[square] +
					(((occupied & tables.bishop_attacks_mask[square])
						* diag_magics[square]) >> tables.bishop_db_shifts[square])];
	}

	/**
	 * Get the mobility of a queen on \a square. This is hashed to
	 * avoid computing it on the fly
	 *
	 * @param[in] square   The queen's location
	 * @param[in] occupied Occupied squares bitboard
	 *
	 * @return A popcnt() of attacksFromQueen()
	 */
	inline int getQueenMobility(int square, uint64 occupied) const
	{
		return getRookMobility(square, occupied)
				+ getBishopMobility(square, occupied);
	}

	/**
	 * Returns the mobility of a rook on \a square. This is hashed to
	 * avoid computing it on the fly
	 *
	 * @param[in] square   The rook's location
	 * @param[in] occupied Occupied squares bitboard
	 *
	 * @return A popcnt() of attacksFromRook()
	 */
	inline int getRookMobility(int square, uint64 occupied) const
	{
		return tables.rook_mobility[tables.rook_offsets[square] +
					(((occupied & tables.rook_attacks_mask[square])
						* rook_magics[square]) >> tables.rook_db_shifts[square])];
	}

	/**
	 * Set default values
	 */
	void setDefault()
	{
		std::memset(pieces, INVALID, 64 * sizeof(piece_t));

		is_init = false;

		for (int i = 0; i < 2; i++)
		{
			occupied[i] = 0;
			kings[i]    = 0;
			rooks[i]    = 0;
			pawns[i]    = 0;
			bishops[i]  = 0;
			knights[i]  = 0;
			queens[i]   = 0;

			kingSq[i] = BAD_SQUARE;
		}

		for (int i = 0; i < MAX_PLY; i++)
		{
			epInfo[i].clear();

			castleRights[i][0] = 0;
			castleRights[i][1] = 0;

			save_hash[i] = 0;
		}

		material[BLACK] = 0;
		material[WHITE] = 0;

		halfMove = -1;
		fullMove = -1;
		toMove = 0;
		ply    = 0;

			hash_input.clear();
	}

	/*
	 * Structure containing en passant information for
	 * this position
	 */
	typedef struct
	{
		int32 target;
		int32 src[2];

		void clear()
		{
			target = src[0] = src[1] = BAD_SQUARE;
		}

	} EnPassant;

	/*
	 * Structure that contains 64-bit integers used
	 * to create a hash signature
	 */
	typedef struct
	{
		uint64 castle_rights[2][2];
		uint64 en_passant[8];
		uint64 piece[2][6][64];
		uint64 to_move;

		/*
		 * Clear all entries
		 */
		void clear()
		{
			for (int i = 0; i < 2; i++)
			{
				castle_rights[WHITE][i] = 0;
				castle_rights[BLACK][i] = 0;
			}

			for (int i = 0; i < 8; i++)
				en_passant[i] = 0;

			for (int i = 0; i < 6; i++)
			{
				for (int j = 0; j < 64; j++)
				{
					piece[0][i][j] = 0;
					piece[1][i][j] = 0;
				}
			}

			to_move = 0;
		}

	} HashInput;


	/*
	 * Note: Limiting data structures to 512 ply means we can only
	 *       support up to 256 moves
	 */
#define DB_LIMIT (MAX_MOVES * 2)

	uint64    bishops[2];
	char      castleRights[DB_LIMIT][2];
	EnPassant epInfo[DB_LIMIT];
	int32     fullMove;
	int32     halfMove;
	HashInput hash_input;
	bool      is_init;
	uint64    kings[2];
	int32     kingSq[2];
	uint64    knights[2];
	uint32    material[2];
	uint64    occupied[2];
	uint64    pawns[2];
	piece_t   pieces[64];
	int32     ply;
	uint64    queens[2];
	uint64    rooks[2];
	EnPassant _saveEP;
	uint64    save_hash[DB_LIMIT];

	const DataTables& tables;

	int32     toMove;
};

#endif
