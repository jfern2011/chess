#include <cstdlib>

#include "position.h"

DataTables tables;

/**
 **********************************************************************
 *
 * @class PositionTest
 *
 * Run unit tests on the Position class
 *
 **********************************************************************
 */
class PositionTest
{
public:
	PositionTest()
	{
	}

	~PositionTest()
	{
	}

	/**
	 * This test places white pawns on E2 and E7, black pawns on D2
	 * and D7, and (in order to have a valid position) a white king
	 * king on G1 and a black king on G8. This routine tests pawn
	 * advances by one and two squares for both white and black, as
	 * well as pawn advances that promote
	 *
	 * @param[in] print If true, print the position 
	 *
	 * @return True on success
	 */
	bool testPawnAdvances(bool print)
	{
		piece_t captured = INVALID, moved = PAWN, promote = INVALID;

		Util::str_v fen;
			fen.push_back("6k1/3pP3/8/8/8/8/3pP3/6K1 w - - 0 1");
			fen.push_back("6k1/3pP3/8/8/8/8/3pP3/6K1 b - - 0 1");

		int side[] = {WHITE, BLACK};

		//=============================================================
		// Pawn advances 2:
		//=============================================================

		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (print) pos.print();

			if (side[i] == WHITE)
			{
				int move = pack(captured,E2,moved,promote,E4);

				AbortIfNot(pos.makeMove(move), false);

				bool success =
					pos.epInfo[pos.ply].target
										== E3      &&
					pos.occupied[WHITE] == Util::createBitboard(E4,
																E7,
																G1) &&

					pos.pawns[WHITE]    == Util::createBitboard(E4,
																E7) &&
					pos.pieces[E2]      == INVALID &&
					pos.pieces[E4]      == PAWN    &&
					pos.ply             == 1       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						   false);
			}
			else
			{
				int move = pack(captured,D7,moved,promote,D5);

				AbortIfNot(pos.makeMove(move), false);

				bool success =
					pos.epInfo[pos.ply].target
										== D6      &&
					pos.occupied[BLACK] == Util::createBitboard(D2,
																D5,
																G8) &&

					pos.pawns[BLACK]    == Util::createBitboard(D2,
																D5) &&
					pos.pieces[D7]      == INVALID &&
					pos.pieces[D5]      == PAWN    &&
					pos.ply             == 1       &&
					pos.fullMove        == 2       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						   false);
			}

			AbortIfNot(pos == saved,
					false);
		}

		//=============================================================
		// Pawn advances 1:
		//=============================================================

		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (print) pos.print();

			if (side[i] == WHITE)
			{
				int move = pack(captured,E2,moved,promote,E3);

				AbortIfNot(pos.makeMove(move), false);

				bool success =
					pos.occupied[WHITE] == Util::createBitboard(E3,
																E7,
																G1) &&

					pos.pawns[WHITE]    == Util::createBitboard(E3,
																E7) &&
					pos.pieces[E2]      == INVALID &&
					pos.pieces[E3]      == PAWN    &&
					pos.ply             == 1       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						   false);
			}
			else
			{
				int move = pack(captured,D7,moved,promote,D6);

				AbortIfNot(pos.makeMove(move), false);

				bool success =
					pos.occupied[BLACK] == Util::createBitboard(D2,
																D6,
																G8) &&

					pos.pawns[BLACK]    == Util::createBitboard(D2,
																D6) &&
					pos.pieces[D7]      == INVALID &&
					pos.pieces[D6]      == PAWN    &&
					pos.ply             == 1       &&
					pos.fullMove        == 2       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						   false);
			}

			AbortIfNot(pos == saved,
					false);
		}

		//=============================================================
		// Pawn advances and promotes:
		//=============================================================

		piece_t promotions[] = { KNIGHT, ROOK, BISHOP, QUEEN };

		for (int k = 0; k < 4; k++)
		{
			promote = promotions[k];

			for (int i = 0; i < 2; i++)
			{
				Position pos(tables, fen[i], false);
				Position saved(pos);

				if (print) pos.print();

				if (side[i] == WHITE)
				{
					int move = pack(captured,E7,moved,promote,E8);

					AbortIfNot(pos.makeMove(move), false);

					uint64 piece64;

					switch (promote)
					{
						case KNIGHT:
							piece64 = pos.knights[WHITE]; break;
						case ROOK:
							piece64 = pos.rooks[WHITE];   break;
						case BISHOP:
							piece64 = pos.bishops[WHITE]; break;
						case QUEEN:
							piece64 = pos.queens[WHITE];  break;
						default:
							std::cout << "Invalid piece: " << promote
								<< std::endl;
							return false;
					}

					bool success =
						pos.occupied[WHITE] == Util::createBitboard(E2,
																	E8,
																	G1) &&

						pos.pawns[WHITE]    == Util::createBitboard(E2) &&
						piece64             == Util::createBitboard(E8) &&
						pos.pieces[E7]      == INVALID &&
						pos.pieces[E8]      == promote &&
						pos.ply             == 1       &&
						pos.toMove          == BLACK;

					AbortIfNot(success && pos.unMakeMove(move),
							   false);
				}
				else
				{
					int move = pack(captured,D2,moved,promote,D1);

					AbortIfNot(pos.makeMove(move), false);

					uint64 piece64;

					switch (promote)
					{
						case KNIGHT:
							piece64 = pos.knights[BLACK]; break;
						case ROOK:
							piece64 = pos.rooks[BLACK];   break;
						case BISHOP:
							piece64 = pos.bishops[BLACK]; break;
						case QUEEN:
							piece64 = pos.queens[BLACK];  break;
						default:
							std::cout << "Invalid piece: " << promote
								<< std::endl;
							return false;
					}

					bool success =
						pos.occupied[BLACK] == Util::createBitboard(D1,
																	D7,
																	G8) &&

						pos.pawns[BLACK]    == Util::createBitboard(D7) &&
						piece64             == Util::createBitboard(D1) &&
						pos.pieces[D2]      == INVALID &&
						pos.pieces[D1]      == promote &&
						pos.ply             == 1       &&
						pos.fullMove        == 2       &&
						pos.toMove          == WHITE;

					AbortIfNot(success && pos.unMakeMove(move),
							   false);
				}

				AbortIfNot(pos == saved,
						false);
			}
		}

		return true;
	}

	/**
	 * This routine tests left and right pawn captures by both sides,
	 * with and without promotions
	 *
	 * This also tests en passant captures
	 *
	 * @param[in] print If true, prints the position 
	 *
	 * @return True on success
	 */
	bool testPawnCaptures(bool print)
	{
		uint64 one = 1;

		Util::str_v fen;
			fen.push_back("7k/pppp4/BPNRQ3/8/8/bpnrq3/PPPP4/7K w - - 0 1");
			fen.push_back("7k/pppp4/BPNRQ3/8/8/bpnrq3/PPPP4/7K b - - 0 1");

		int side[] = {WHITE, BLACK};

		SQUARE from[2][4] = {{A2, B2, C2, D2},
							 {A7, B7, C7, D7}};

		SQUARE to_r[2][4] = {{B3, C3, D3, E3},
							 {B6, C6, D6, E6}};

		SQUARE to_l[2][4] = {{B3, A3, B3, C3},
							 {B6, A6, B6, C6}};

		piece_t captures_r[] =
			{ PAWN, KNIGHT, ROOK, QUEEN  };
		piece_t captures_l[] =
			{ PAWN, BISHOP, PAWN, KNIGHT };

		/*
		 * Pawn captures to the right:
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			uint64 occ[2] = { pos.occupied[WHITE], pos.occupied[BLACK] };

			for (int j = 0; j < 4; j++)
			{
				if (side[i] == WHITE)
				{
					int move = pack(captures_r[j],
									from[0][j],
									PAWN, INVALID,
									to_r[0][j]);

					AbortIfNot(pos.makeMove(move),
							false);

					bool success = true;

					switch (captures_r[j])
					{
					case PAWN:
						success = success &&
							pos.pawns[BLACK]    ==
								Util::createBitboard(A7,B7,C7,D7)    &&
							pos.occupied[WHITE] ==
								(occ[0] | (one << B3)) ^ (one << A2) &&
							pos.occupied[BLACK] ==
								(occ[1] ^ (one << B3));
						break;
					case KNIGHT:
						success = success &&
							pos.knights[BLACK]  == 0 &&
							pos.occupied[WHITE] ==
								(occ[0] | (one << C3)) ^ (one << B2) &&
							pos.occupied[BLACK] ==
								(occ[1] ^ (one << C3));
						break;
					case ROOK:
						success = success &&
							pos.rooks[BLACK]   == 0 &&
							pos.occupied[WHITE] ==
								(occ[0] | (one << D3)) ^ (one << C2) &&
							pos.occupied[BLACK] ==
								(occ[1] ^ (one << D3));
						break; 
					case QUEEN:
						success = success &&
							pos.queens[BLACK]  == 0 &&
							pos.occupied[WHITE] ==
								(occ[0] | (one << E3)) ^ (one << D2) &&
							pos.occupied[BLACK] ==
								(occ[1] ^ (one << E3));
						break;
					default:
						std::cout << "Invalid piece: " << captures_r[j]
							<< std::endl;
						return false;
					}

					success = success &&
						pos.pawns[WHITE] ==
							(Util::createBitboard(A2,B2,B6,C2,D2)
								^ (one << from[0][j])
								^ (one << to_r[0][j]))            &&
						pos.pieces[from[0][j]] == INVALID         &&
						pos.pieces[to_r[0][j]] == PAWN            &&
						pos.ply                == 1               &&
						pos.toMove             == BLACK;

					AbortIfNot(success && pos.unMakeMove(move),
							false);
				}
				else
				{
					int move = pack(captures_r[j],
									from[1][j],
									PAWN, INVALID,
									to_r[1][j]);

					AbortIfNot(pos.makeMove(move),
							false);

					bool success = true;

					switch (captures_r[j])
					{
					case PAWN:
						success = success &&
							pos.pawns[WHITE]    ==
								Util::createBitboard(A2,B2,C2,D2)    &&
							pos.occupied[BLACK] ==
								(occ[1] | (one << B6)) ^ (one << A7) &&
							pos.occupied[WHITE] ==
								(occ[0] ^ (one << B6));
						break;
					case KNIGHT:
						success = success &&
							pos.knights[WHITE]  == 0 &&
							pos.occupied[BLACK] ==
								(occ[1] | (one << C6)) ^ (one << B7) &&
							pos.occupied[WHITE] ==
								(occ[0] ^ (one << C6));
						break;
					case ROOK:
						success = success &&
							pos.rooks[WHITE]   == 0 &&
							pos.occupied[BLACK] ==
								(occ[1] | (one << D6)) ^ (one << C7) &&
							pos.occupied[WHITE] ==
								(occ[0] ^ (one << D6));
						break; 
					case QUEEN:
						success = success &&
							pos.queens[WHITE]  == 0 &&
							pos.occupied[BLACK] ==
								(occ[1] | (one << E6)) ^ (one << D7) &&
							pos.occupied[WHITE] ==
								(occ[0] ^ (one << E6));
						break;
					default:
						std::cout << "Invalid piece: " << captures_r[j]
							<< std::endl;
						return false;
					}

					success = success &&
						pos.pawns[BLACK] ==
							(Util::createBitboard(A7,B7,B3,C7,D7)
								^ (one << from[1][j]) ^ to_r[1][j]) &&
						pos.pieces[from[1][j]] == INVALID           &&
						pos.pieces[to_r[1][j]] == PAWN              &&
						pos.ply                == 1                 &&
						pos.fullMove           == 2                 &&
						pos.toMove             == WHITE;

					AbortIfNot(pos.unMakeMove(move),
							false);
				}

				AbortIfNot(pos == saved,
					false);
			}
		}

		/*
		 * Pawn captures to the left:
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			uint64 occ[2] = { pos.occupied[WHITE], pos.occupied[BLACK] };

			for (int j = 0; j < 4; j++)
			{
				if (side[i] == WHITE)
				{
					int move = pack(captures_l[j],
									from[0][j],
									PAWN, INVALID,
									to_l[0][j]);

					AbortIfNot(pos.makeMove(move),
							false);

					bool success = true;

					switch (captures_l[j])
					{
					case PAWN:
						// Ignore A-pawn since it can't capture in this
						// direction:
						if (from[0][j] == A2)
							break;
						success = success &&
							pos.pawns[BLACK]    ==
								Util::createBitboard(A7,B7,C7,D7)    &&
							pos.occupied[WHITE] ==
								(occ[0] | (one << B3)) ^ (one << C2) &&
							pos.occupied[BLACK] ==
								(occ[1] ^ (one << B3));
						break;
					case KNIGHT:
						success = success &&
							pos.knights[BLACK]  == 0 &&
							pos.occupied[WHITE] ==
								(occ[0] | (one << C3)) ^ (one << D2) &&
							pos.occupied[BLACK] ==
								(occ[1] ^ (one << C3));
						break;
					case BISHOP:
						success = success &&
							pos.bishops[BLACK]  == 0 &&
							pos.occupied[WHITE] ==
								(occ[0] | (one << A3)) ^ (one << B2) &&
							pos.occupied[BLACK] ==
								(occ[1] ^ (one << A3));
						break; 
					default:
						std::cout << "Invalid piece: " << captures_r[j]
							<< std::endl;
						return false;
					}

					success = success &&
						pos.pawns[WHITE] ==
							(Util::createBitboard(A2,B2,B6,C2,D2)
								^ (one << from[0][j])
								^ (one << to_l[0][j]))            &&
						pos.pieces[from[0][j]] == INVALID         &&
						pos.pieces[to_l[0][j]] == PAWN            &&
						pos.ply                == 1               &&
						pos.toMove             == BLACK;

					AbortIfNot(success && pos.unMakeMove(move),
							false);
				}
				else
				{
					int move = pack(captures_r[j],
									from[1][j],
									PAWN, INVALID,
									to_r[1][j]);

					AbortIfNot(pos.makeMove(move),
							false);

					bool success = true;

					switch (captures_l[j])
					{
					case PAWN:
						// Ignore A-pawn since it can't capture in this
						// direction:
						if (from[0][j] == A7)
							break;
						success = success &&
							pos.pawns[WHITE]    ==
								Util::createBitboard(A2,B2,C2,D2)    &&
							pos.occupied[BLACK] ==
								(occ[1] | (one << B6)) ^ (one << C7) &&
							pos.occupied[WHITE] ==
								(occ[0] ^ (one << B6));
						break;
					case KNIGHT:
						success = success &&
							pos.knights[WHITE]  == 0 &&
							pos.occupied[BLACK] ==
								(occ[1] | (one << C6)) ^ (one << D7) &&
							pos.occupied[WHITE] ==
								(occ[0] ^ (one << C6));
						break;
					case BISHOP:
						success = success &&
							pos.bishops[WHITE]   == 0 &&
							pos.occupied[BLACK] ==
								(occ[1] | (one << A6)) ^ (one << B7) &&
							pos.occupied[WHITE] ==
								(occ[0] ^ (one << A6));
						break; 
					default:
						std::cout << "Invalid piece: " << captures_l[j]
							<< std::endl;
						return false;
					}

					success = success &&
						pos.pawns[BLACK] ==
							(Util::createBitboard(A7,B7,B3,C7,D7)
								^ (one << from[1][j]) ^ to_l[1][j]) &&
						pos.pieces[from[1][j]] == INVALID           &&
						pos.pieces[to_l[1][j]] == PAWN              &&
						pos.ply                == 1                 &&
						pos.fullMove           == 2                 &&
						pos.toMove             == WHITE;

					AbortIfNot(pos.unMakeMove(move),
							false);
				}

				AbortIfNot(pos == saved,
					false);
			}
		}

		/*
		 * Pawn captures to the right and promotes:
		 */

		fen[0] = "1bnrq2k/PPPP4/8/8/8/8/pppp4/1BNRQ2K w - - 0 1";
		fen[1] = "1bnrq2k/PPPP4/8/8/8/8/pppp4/1BNRQ2K b - - 0 1";

		piece_t promotions[] =
			{ KNIGHT, BISHOP, ROOK, QUEEN };

		from[0][0] = A7; from[0][1] = B7;
		from[0][2] = C7; from[0][3] = D7;

		from[1][0] = A2; from[1][1] = B2;
		from[1][2] = C2; from[1][3] = D2;

		to_r[0][0] = B8; to_r[0][1] = C8;
		to_r[0][2] = D8; to_r[0][3] = E8;

		to_r[1][0] = B1; to_r[1][1] = C1;
		to_r[1][2] = D1; to_r[1][3] = E1;

		captures_r[0] = BISHOP;
		captures_r[1] = KNIGHT;
		captures_r[2] = ROOK;
		captures_r[3] = QUEEN ;

		for (int k = 0; k < 4; k++)
		{
			piece_t promote = promotions[k];

			for (int i = 0; i < 2; i++)
			{
				Position pos(tables, fen[i], false);
				Position saved(pos);

				uint64 occ[2] = { pos.occupied[WHITE], pos.occupied[BLACK] };

				for (int j = 0; j < 4; j++)
				{
					if (side[i] == WHITE)
					{
						int move = pack(captures_r[j],
										from[0][j],
										PAWN, promote,
										to_r[0][j]);

						AbortIfNot(pos.makeMove(move),
								false);

						bool success = true;

						switch (captures_r[j])
						{
						case BISHOP:
							success = success &&
								pos.bishops[BLACK]  == 0 &&
								pos.occupied[WHITE] ==
									(occ[0] | (one << B8)) ^ (one << A7) &&
								pos.occupied[BLACK] ==
									(occ[1] ^ (one << B8));
							break;
						case KNIGHT:
							success = success &&
								pos.knights[BLACK]  == 0 &&
								pos.occupied[WHITE] ==
									(occ[0] | (one << C8)) ^ (one << B7) &&
								pos.occupied[BLACK] ==
									(occ[1] ^ (one << C8));
							break;
						case ROOK:
							success = success &&
								pos.rooks[BLACK]   == 0 &&
								pos.occupied[WHITE] ==
									(occ[0] | (one << D8)) ^ (one << C7) &&
								pos.occupied[BLACK] ==
									(occ[1] ^ (one << D8));
							break; 
						case QUEEN:
							success = success &&
								pos.queens[BLACK]  == 0 &&
								pos.occupied[WHITE] ==
									(occ[0] | (one << E8)) ^ (one << D7) &&
								pos.occupied[BLACK] ==
									(occ[1] ^ (one << E8));
							break;
						default:
							std::cout << "Invalid piece: " << captures_r[j]
								<< std::endl;
							return false;
						}

						switch (promote)
						{
						case KNIGHT:
							success = success &&
								pos.knights[WHITE] ==
									Util::createBitboard(C1,to_r[0][j]);
							break;
						case BISHOP:
							success = success &&
								pos.bishops[WHITE] ==
									Util::createBitboard(B1,to_r[0][j]);
							break;
						case ROOK:
							success = success &&
								pos.rooks[WHITE] ==
									Util::createBitboard(D1,to_r[0][j]);
							break;
						case QUEEN:
							success = success &&
								pos.queens[WHITE] ==
									Util::createBitboard(E1,to_r[0][j]);
							break;
						default:
							std::cout << "Invalid piece: " << promote
								<< std::endl;
							return false;
						}

						success = success &&
							pos.pawns[WHITE] ==
								(Util::createBitboard(A7,B7,C7,D7)
									^ (one << from[0][j]))            &&

							pos.pieces[from[0][j]] == INVALID         &&
							pos.pieces[to_r[0][j]] == promote         &&
							pos.ply                == 1               &&
							pos.toMove             == BLACK;

						AbortIfNot(success && pos.unMakeMove(move),
								false);
					}
					else
					{
						int move = pack(captures_r[j],
										from[1][j],
										PAWN, promote,
										to_r[1][j]);

						AbortIfNot(pos.makeMove(move),
								false);

						bool success = true;

						switch (captures_r[j])
						{
						case BISHOP:
							success = success &&
								pos.bishops[WHITE]  == 0 &&
								pos.occupied[BLACK] ==
									(occ[1] | (one << B1)) ^ (one << A2) &&
								pos.occupied[WHITE] ==
									(occ[0] ^ (one << B1));
							break;
						case KNIGHT:
							success = success &&
								pos.knights[WHITE]  == 0 &&
								pos.occupied[BLACK] ==
									(occ[1] | (one << C1)) ^ (one << B2) &&
								pos.occupied[WHITE] ==
									(occ[0] ^ (one << C1));
							break;
						case ROOK:
							success = success &&
								pos.rooks[WHITE]   == 0 &&
								pos.occupied[BLACK] ==
									(occ[1] | (one << D1)) ^ (one << C2) &&
								pos.occupied[WHITE] ==
									(occ[0] ^ (one << D1));
							break; 
						case QUEEN:
							success = success &&
								pos.queens[WHITE]  == 0 &&
								pos.occupied[BLACK] ==
									(occ[1] | (one << E1)) ^ (one << D2) &&
								pos.occupied[WHITE] ==
									(occ[0] ^ (one << E1));
							break;
						default:
							std::cout << "Invalid piece: " << captures_r[j]
								<< std::endl;
							return false;
						}

						switch (promote)
						{
						case KNIGHT:
							success = success &&
								pos.knights[BLACK] ==
									Util::createBitboard(B1,to_r[0][j]);
							break;
						case BISHOP:
							success = success &&
								pos.bishops[BLACK] ==
									Util::createBitboard(C1,to_r[0][j]);
							break;
						case ROOK:
							success = success &&
								pos.rooks[BLACK] ==
									Util::createBitboard(D1,to_r[0][j]);
							break;
						case QUEEN:
							success = success &&
								pos.queens[BLACK] ==
									Util::createBitboard(E1,to_r[0][j]);
							break;
						default:
							std::cout << "Invalid piece: " << promote
								<< std::endl;
							return false;
						}

						success = success &&
							pos.pawns[BLACK] ==
								(Util::createBitboard(A2,B2,B2,C2,D2)
									^ (one << from[1][j]))              &&
							pos.pieces[from[1][j]] == INVALID           &&
							pos.pieces[to_r[1][j]] == promote           &&
							pos.ply                == 1                 &&
							pos.fullMove           == 2                 &&
							pos.toMove             == WHITE;

						AbortIfNot(pos.unMakeMove(move),
								false);
					}

					AbortIfNot(pos == saved,
						false);
				}
			}
		}

		/*
		 * Left and right en passant captures
		 */
		fen[0] = "4k3/8/8/3PpP2/3pPp2/8/8/4K3 w - e6 0 1";
		fen[1] = "4k3/8/8/3PpP2/3pPp2/8/8/4K3 b - e3 0 1";

		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				int move = pack(PAWN,D5,PAWN,INVALID,E6);

				uint64 occ[2] = { pos.occupied[WHITE], pos.occupied[BLACK] };

				AbortIfNot(pos.makeMove(move),
						false);

				bool success =
					pos.epInfo[pos.ply].src[0] == BAD_SQUARE &&
					pos.epInfo[pos.ply].src[1] == BAD_SQUARE &&
					pos.epInfo[pos.ply].target == BAD_SQUARE;

				success = success &&
					pos.occupied[WHITE] ==
						Util::createBitboard(E4,
											 E6,
											 F5,
											 E1);
				success = success &&
					pos.occupied[BLACK] ==
						Util::createBitboard(D4,
											 F4,
											 E8);

				success = success &&
					pos.pawns[WHITE] ==
						pos.occupied[WHITE] ^ pos.kings[WHITE];

				success = success &&
					pos.pawns[BLACK] ==
						pos.occupied[BLACK] ^ pos.kings[BLACK];

				success = success && 
					pos.pieces[D5] == INVALID &&
					pos.pieces[E6] == PAWN    &&
					pos.pieces[E5] == INVALID &&
					pos.ply        == 1;

				AbortIfNot(success && pos.unMakeMove(move),
						false);

				AbortIfNot(pos == saved,
						false);

				move = pack(PAWN,F5,PAWN,INVALID,E6);

				AbortIfNot(pos.makeMove(move),false);

				success =
					pos.epInfo[pos.ply].src[0] == BAD_SQUARE &&
					pos.epInfo[pos.ply].src[1] == BAD_SQUARE &&
					pos.epInfo[pos.ply].target == BAD_SQUARE;

				success = success &&
					pos.occupied[WHITE] ==
						Util::createBitboard(E4,
											 E6,
											 D5,
											 E1);
				success = success &&
					pos.occupied[BLACK] ==
						Util::createBitboard(D4,
											 F4,
											 E8);

				success = success &&
					pos.pawns[WHITE] ==
						pos.occupied[WHITE] ^ pos.kings[WHITE];

				success = success &&
					pos.pawns[BLACK] ==
						pos.occupied[BLACK] ^ pos.kings[BLACK];

				success = success && 
					pos.pieces[E5] == INVALID &&
					pos.pieces[E6] == PAWN    &&
					pos.pieces[F5] == INVALID &&
					pos.ply        == 1;

				AbortIfNot(success && pos.unMakeMove(move),
						false);

				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				int move = pack(PAWN,D4,PAWN,INVALID,E3);

				uint64 occ[2] = { pos.occupied[WHITE], pos.occupied[BLACK] };

				AbortIfNot(pos.makeMove(move),
						false);

				bool success =
					pos.epInfo[pos.ply].src[0] == BAD_SQUARE &&
					pos.epInfo[pos.ply].src[1] == BAD_SQUARE &&
					pos.epInfo[pos.ply].target == BAD_SQUARE;

				success = success &&
					pos.occupied[WHITE] ==
						Util::createBitboard(D5,
											 F5,
											 E1);
				success = success &&
					pos.occupied[BLACK] ==
						Util::createBitboard(E3,
											 E5,
											 F4,
											 E8);

				success = success &&
					pos.pawns[WHITE] ==
						pos.occupied[WHITE] ^ pos.kings[WHITE];

				success = success &&
					pos.pawns[BLACK] ==
						pos.occupied[BLACK] ^ pos.kings[BLACK];

				success = success && 
					pos.pieces[D4] == INVALID &&
					pos.pieces[E3] == PAWN    &&
					pos.pieces[E4] == INVALID &&
					pos.fullMove   == 2       &&
					pos.ply        == 1;

				AbortIfNot(success && pos.unMakeMove(move),
						false);

				AbortIfNot(pos == saved,
						false);

				move = pack(PAWN,F4,PAWN,INVALID,E3);

				AbortIfNot(pos.makeMove(move),false);

				success =
					pos.epInfo[pos.ply].src[0] == BAD_SQUARE &&
					pos.epInfo[pos.ply].src[1] == BAD_SQUARE &&
					pos.epInfo[pos.ply].target == BAD_SQUARE;

				success = success &&
					pos.occupied[WHITE] ==
						Util::createBitboard(D5,
											 F5,
											 E1);

				success = success &&
					pos.occupied[BLACK] ==
						Util::createBitboard(D4,
											 E3,
											 E5,
											 E8);

				success = success &&
					pos.pawns[WHITE] ==
						pos.occupied[WHITE] ^ pos.kings[WHITE];

				success = success &&
					pos.pawns[BLACK] ==
						pos.occupied[BLACK] ^ pos.kings[BLACK];

				success = success && 
					pos.pieces[E4] == INVALID &&
					pos.pieces[E3] == PAWN    &&
					pos.pieces[F4] == INVALID &&
					pos.fullMove   == 2       &&
					pos.ply        == 1;

				AbortIfNot(success && pos.unMakeMove(move),
						false);

				AbortIfNot(pos == saved,
						false);
			}
		}

		return true;
	}

	/**
	 * Test rook captures and non-captures for both sides
	 *
	 * @return True on success
	 */
	bool testRookMoves()
	{
		Util::str_v fen;
		fen.push_back("4k3/8/1P1r4/8/8/1p1R4/8/4K3 w - - 0 1");
		fen.push_back("4k3/8/1P1r4/8/8/1p1R4/8/4K3 b - - 0 1");

		uint64 one = 1;

		int side[] = {WHITE, BLACK};

		/*
		 * Rook captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				int move = pack(PAWN,D3,ROOK,INVALID,B3);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,B3,B6) &&
					pos.occupied[BLACK] ==
						Util::createBitboard(D6,E8)    &&
					pos.pawns[BLACK]    == 0           &&
					pos.pieces[B3]      == ROOK        &&
					pos.pieces[D3]      == INVALID     &&
					pos.ply             == 1           &&
					pos.rooks[WHITE]    ==
						Util::createBitboard(B3)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				int move = pack(PAWN,D6,ROOK,INVALID,B6);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,B3,B6) &&
					pos.occupied[WHITE] ==
						Util::createBitboard(D3,E1)    &&
					pos.pawns[WHITE]    == 0           &&
					pos.pieces[B6]      == ROOK        &&
					pos.pieces[D6]      == INVALID     &&
					pos.ply             == 1           &&
					pos.rooks[BLACK]    ==
						Util::createBitboard(B6)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		/*
		 * Rook non-captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				int move = pack(INVALID,D3,ROOK,INVALID,D2);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,D2,B6) &&
					pos.pieces[D2]      == ROOK        &&
					pos.pieces[D3]      == INVALID     &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.rooks[WHITE]    ==
						Util::createBitboard(D2)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				int move = pack(INVALID,D6,ROOK,INVALID,C6);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,B3,C6) &&
					pos.pieces[C6]      == ROOK        &&
					pos.pieces[D6]      == INVALID     &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.fullMove        == 2           &&
					pos.rooks[BLACK]    ==
						Util::createBitboard(C6)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		return true;
	}

	/**
	 * Test queen captures and non-captures for both sides
	 *
	 * @return True on success
	 */
	bool testQueenMoves()
	{
		Util::str_v fen;
		fen.push_back("4k3/8/1P1q4/8/8/1p1Q4/8/4K3 w - - 0 1");
		fen.push_back("4k3/8/1P1q4/8/8/1p1Q4/8/4K3 b - - 0 1");

		uint64 one = 1;

		int side[] = {WHITE, BLACK};

		/*
		 * Queen captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				int move = pack(PAWN,D3,QUEEN,INVALID,B3);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,B3,B6) &&
					pos.occupied[BLACK] ==
						Util::createBitboard(D6,E8)    &&
					pos.pawns[BLACK]    == 0           &&
					pos.pieces[B3]      == QUEEN       &&
					pos.pieces[D3]      == INVALID     &&
					pos.ply             == 1           &&
					pos.queens[WHITE]   ==
						Util::createBitboard(B3)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				int move = pack(PAWN,D6,QUEEN,INVALID,B6);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,B3,B6) &&
					pos.occupied[WHITE] ==
						Util::createBitboard(D3,E1)    &&
					pos.pawns[WHITE]    == 0           &&
					pos.pieces[B6]      == QUEEN       &&
					pos.pieces[D6]      == INVALID     &&
					pos.ply             == 1           &&
					pos.fullMove        == 2           &&
					pos.queens[BLACK]   ==
						Util::createBitboard(B6)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		/*
		 * Queen non-captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				int move = pack(INVALID,D3,QUEEN,INVALID,D2);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,D2,B6) &&
					pos.pieces[D2]      == QUEEN       &&
					pos.pieces[D3]      == INVALID     &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.queens[WHITE]   ==
						Util::createBitboard(D2)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				int move = pack(INVALID,D6,QUEEN,INVALID,C6);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,B3,C6) &&
					pos.pieces[C6]      == QUEEN       &&
					pos.pieces[D6]      == INVALID     &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.fullMove        == 2           &&
					pos.queens[BLACK]   ==
						Util::createBitboard(C6)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		return true;
	}

	/**
	 * Test knight captures and non-captures for both sides
	 *
	 * @return True on success
	 */
	bool testKnightMoves()
	{
		Util::str_v fen;
		fen.push_back("4k3/8/1P6/3n4/3N4/1p6/8/4K3 w - - 0 1");
		fen.push_back("4k3/8/1P6/3n4/3N4/1p6/8/4K3 b - - 0 1");

		uint64 one = 1;

		int side[] = {WHITE, BLACK};

		/*
		 * Knight captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				int move = pack(PAWN,D4,KNIGHT,INVALID,B3);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,B3,B6) &&
					pos.occupied[BLACK] ==
						Util::createBitboard(D5,E8)    &&
					pos.pawns[BLACK]    == 0           &&
					pos.pieces[B3]      == KNIGHT      &&
					pos.pieces[D4]      == INVALID     &&
					pos.ply             == 1           &&
					pos.knights[WHITE]  ==
						Util::createBitboard(B3)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				int move = pack(PAWN,D5,KNIGHT,INVALID,B6);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,B3,B6) &&
					pos.occupied[WHITE] ==
						Util::createBitboard(D4,E1)    &&
					pos.pawns[WHITE]    == 0           &&
					pos.pieces[B6]      == KNIGHT      &&
					pos.pieces[D5]      == INVALID     &&
					pos.ply             == 1           &&
					pos.fullMove        == 2           &&
					pos.knights[BLACK]  ==
						Util::createBitboard(B6)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		/*
		 * Knight non-captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				int move = pack(INVALID,D4,KNIGHT,INVALID,B5);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,B5,B6) &&
					pos.pieces[B5]      == KNIGHT      &&
					pos.pieces[D4]      == INVALID     &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.knights[WHITE]  ==
						Util::createBitboard(B5)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				int move = pack(INVALID,D5,KNIGHT,INVALID,B4);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,B4,B3) &&
					pos.pieces[B4]      == KNIGHT      &&
					pos.pieces[D5]      == INVALID     &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.fullMove        == 2           &&
					pos.knights[BLACK]  ==
						Util::createBitboard(B4)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		return true;
	}

	/**
	 * Test bishop captures and non-captures for both sides
	 *
	 * @return True on success
	 */
	bool testBishopMoves()
	{
		Util::str_v fen;
		fen.push_back("4k3/8/1P6/3B4/3b4/1p6/8/4K3 w - - 0 1");
		fen.push_back("4k3/8/1P6/3B4/3b4/1p6/8/4K3 b - - 0 1");

		uint64 one = 1;

		int side[] = {WHITE, BLACK};

		/*
		 * Bishop captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				const SQUARE from = D5;
				const SQUARE to   = B3;

				int move = pack(PAWN,from,BISHOP,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,B3,B6) &&
					pos.occupied[BLACK] ==
						Util::createBitboard(D4,E8)    &&
					pos.pawns[BLACK]    == 0           &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == BISHOP      &&
					pos.ply             == 1           &&
					pos.bishops[WHITE]  ==
						Util::createBitboard(to)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				const SQUARE from = D4;
				const SQUARE to   = B6;

				int move = pack(PAWN,from,BISHOP,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,B3,B6) &&
					pos.occupied[WHITE] ==
						Util::createBitboard(D5,E1)    &&
					pos.pawns[WHITE]    == 0           &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == BISHOP      &&
					pos.ply             == 1           &&
					pos.fullMove        == 2           &&
					pos.bishops[BLACK]  ==
						Util::createBitboard(B6)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		/*
		 * Bishop non-captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				const SQUARE from = D5;
				const SQUARE to   = E6;

				int move = pack(INVALID,from,BISHOP,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(E1,E6,B6) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == BISHOP      &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.bishops[WHITE]  ==
						Util::createBitboard(to)       &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				const SQUARE from = D4;
				const SQUARE to   = E3;

				int move = pack(INVALID,from,BISHOP,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(E8,E3,B3) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == BISHOP      &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.fullMove        == 2           &&
					pos.bishops[BLACK]  ==
						Util::createBitboard(to)       &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		return true;
	}

	/**
	 * Test rook captures and non-captures for both sides, as well
	 * as castling
	 *
	 * @return True on success
	 */
	bool testKingMoves()
	{
		Util::str_v fen;
		fen.push_back("r3k2r/4P3/8/8/8/8/4p3/R3K2R w KQkq - 0 1");
		fen.push_back("r3k2r/4P3/8/8/8/8/4p3/R3K2R b KQkq - 0 1");

		uint64 one = 1;

		int side[] = {WHITE, BLACK};

		/*
		 * King captures
		 */
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				const SQUARE from = E1;
				const SQUARE to   = E2;

				int move = pack(PAWN,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(A1,H1,E2,E7) &&
					pos.occupied[BLACK] ==
						Util::createBitboard(A8,H8,E8) &&
					pos.pawns[BLACK]    == 0           &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.ply             == 1           &&
					pos.castleRights[pos.ply][WHITE]
										== 0           &&
					pos.castleRights[pos.ply][BLACK]
										== 3           &&
					pos.kings[WHITE]    == (one << E2) &&
					pos.kingSq[WHITE]   == E2          &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				const SQUARE from = E8;
				const SQUARE to   = E7;

				int move = pack(PAWN,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(A8,H8,E7,E2) &&
					pos.occupied[WHITE] ==
						Util::createBitboard(A1,H1,E1) &&
					pos.pawns[WHITE]    == 0           &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.ply             == 1           &&
					pos.fullMove        == 2           &&
					pos.castleRights[pos.ply][BLACK]
										== 0           &&
					pos.castleRights[pos.ply][WHITE]
										== 3           &&
					pos.kings[BLACK]    == (one << E7) &&
					pos.kingSq[BLACK]   == E7          &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		/*
		 * King non-captures
		 */
		fen.clear();
		fen.push_back("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
		fen.push_back("4k3/8/8/8/8/8/8/4K3 b - - 0 1");
		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				const SQUARE from = E1;
				const SQUARE to   = E2;

				int move = pack(INVALID,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] == (one << E2) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.castleRights[pos.ply][WHITE]
										== 0           &&
					pos.kings[WHITE]    == (one << E2) &&
					pos.kingSq[WHITE]   == E2          &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				const SQUARE from = E8;
				const SQUARE to   = E7;

				int move = pack(INVALID,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] == (one << E7) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.fullMove        == 2           &&
					pos.castleRights[pos.ply][BLACK]
										== 0           &&
					pos.kings[BLACK]    == (one << E7) &&
					pos.kingSq[BLACK]   == E7          &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		/*
		 * King castles long
		 */
		fen.clear();
		fen.push_back("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
		fen.push_back("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");

		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				const SQUARE from = E1;
				const SQUARE to   = C1;

				int move = pack(INVALID,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(C1,D1,H1) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.pieces[D1]      == ROOK        &&
					pos.pieces[A1]      == INVALID     &&
					pos.rooks[WHITE]    ==
						Util::createBitboard(D1,H1)    &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.castleRights[pos.ply][WHITE]
										== 0           &&
					pos.castleRights[pos.ply][BLACK]
										== 3           &&
					pos.kings[WHITE]    == (one << C1) &&
					pos.kingSq[WHITE]   == C1          &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				const SQUARE from = E8;
				const SQUARE to   = C8;

				int move = pack(INVALID,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(C8,D8,H8) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.pieces[D8]      == ROOK        &&
					pos.pieces[A8]      == INVALID     &&
					pos.rooks[BLACK]    ==
						Util::createBitboard(D8,H8)    &&
					pos.fullMove        == 2           &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.castleRights[pos.ply][BLACK]
										== 0           &&
					pos.castleRights[pos.ply][WHITE]
										== 3           &&
					pos.kings[BLACK]    == (one << C8) &&
					pos.kingSq[BLACK]   == C8          &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		/*
		 * King castles short
		 */
		fen.clear();
		fen.push_back("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
		fen.push_back("r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1");

		for (int i = 0; i < 2; i++)
		{
			Position pos(tables, fen[i], false);
			Position saved(pos);

			if (side[i] == WHITE)
			{
				const SQUARE from = E1;
				const SQUARE to   = G1;

				int move = pack(INVALID,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[WHITE] ==
						Util::createBitboard(G1,F1,A1) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.pieces[F1]      == ROOK        &&
					pos.pieces[H1]      == INVALID     &&
					pos.rooks[WHITE]    ==
						Util::createBitboard(F1,A1)    &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.castleRights[pos.ply][WHITE]
										== 0           &&
					pos.castleRights[pos.ply][BLACK]
										== 3           &&
					pos.kings[WHITE]    == (one << G1) &&
					pos.kingSq[WHITE]   == G1          &&
					pos.toMove          == BLACK;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
			else
			{
				const SQUARE from = E8;
				const SQUARE to   = G8;

				int move = pack(INVALID,from,KING,INVALID,to);
				AbortIfNot(pos.makeMove(move),
						false);

				bool success = 
					pos.occupied[BLACK] ==
						Util::createBitboard(G8,F8,A8) &&
					pos.pieces[from]    == INVALID     &&
					pos.pieces[to]      == KING        &&
					pos.pieces[F8]      == ROOK        &&
					pos.pieces[H8]      == INVALID     &&
					pos.rooks[BLACK]    ==
						Util::createBitboard(F8,A8)    &&
					pos.fullMove        == 2           &&
					pos.ply             == 1           &&
					pos.halfMove        == 1           &&
					pos.castleRights[pos.ply][BLACK]
										== 0           &&
					pos.castleRights[pos.ply][WHITE]
										== 3           &&
					pos.kings[BLACK]    == (one << G8) &&
					pos.kingSq[BLACK]   == G8          &&
					pos.toMove          == WHITE;

				AbortIfNot(success && pos.unMakeMove(move),
						false);
				AbortIfNot(pos == saved,
						false);
			}
		}

		return true;
	}

	/**
	 * Test the make() and unmake() Position methods, taking into
	 * consideration every possible type of move. For pawns, this
	 * includes single advances, double advances, advances with
	 * promotion, captures, captures with promotion, and en
	 * passant captures. For all other pieces, this includes both
	 * captures and non-captures, except for the king, which also
	 * includes short and long castles
	 *
	 * @return True on success
	 */
	static bool testMakeUndo()
	{
		PositionTest test;

		return
			test.testPawnAdvances(false) &&
			test.testPawnCaptures(false) &&
			test.testRookMoves()         &&
			test.testKnightMoves()       &&
			test.testBishopMoves()       &&
			test.testQueenMoves()        &&
			test.testKingMoves();
	}
};

int main()
{
	if (!PositionTest::testMakeUndo())
	{
		std::cout << "Test failed."
			<< std::endl;
					return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
