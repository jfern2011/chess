#ifndef __EVAL_H__
#define __EVAL_H__

#include "movegen.h"

#define MATE_SCORE 1000000

class Evaluator
{

public:

	/**
	 * A mapping from piece enumeration to its value:
	 */
	static const int piece_value[7];

	typedef enum
	{
		PAWN_VALUE   = 1000,
		KNIGHT_VALUE = 3250,
		BISHOP_VALUE = 3250,
		ROOK_VALUE   = 5000,
		QUEEN_VALUE  = 9750

	} value_t;

	/**
	 * Constructor
	 *
	 * @param[in] movegen A MoveGen object
	 */
	Evaluator(const MoveGen& movegen)
		: _movegen(movegen)
	{
		// configure random evaluation
	}

	~Evaluator()
	{
	}

	int32 evaluate(const Position& pos) const
	{
		return evaluateMaterial(pos) ;//+
						//evaluateMobility(pos);
	}

	inline int32 evaluateMaterial(const Position& pos) const
	{
		const int whiteMaterial =
			_movegen.popCnt64(pos.pawns[WHITE])   * PAWN_VALUE   + 
			_movegen.popCnt64(pos.knights[WHITE]) * KNIGHT_VALUE + 
			_movegen.popCnt64(pos.bishops[WHITE]) * BISHOP_VALUE + 
			_movegen.popCnt64(pos.rooks[WHITE])   * ROOK_VALUE   +
			_movegen.popCnt64(pos.queens[WHITE])  * QUEEN_VALUE;

		const int blackMaterial =
			_movegen.popCnt64(pos.pawns[BLACK])   * PAWN_VALUE   + 
			_movegen.popCnt64(pos.knights[BLACK]) * KNIGHT_VALUE + 
			_movegen.popCnt64(pos.bishops[BLACK]) * BISHOP_VALUE + 
			_movegen.popCnt64(pos.rooks[BLACK])   * ROOK_VALUE   +
			_movegen.popCnt64(pos.queens[BLACK])  * QUEEN_VALUE;

		return
			whiteMaterial - blackMaterial;
	}

	inline int32 evaluateMobility(const Position& pos) const
	{
		uint64 occupied = pos.occupied[0] | pos.occupied[1];
		register int score = 0;

		const int WEIGHT = PAWN_VALUE/4;

		/*
		 * Compute rook mobility score:
		 */
		uint64 temp = pos.rooks[WHITE];
		while (temp)
		{
			const int square = _movegen.getMSB64(temp);
			score +=
				 pos.getRookMobility(square, occupied);

			_movegen.clearBit64(square, temp);
		}

		temp = pos.rooks[BLACK];
		while (temp)
		{
			const int square = _movegen.getMSB64(temp);
			score -=
				 pos.getRookMobility(square, occupied);
				 
			_movegen.clearBit64(square, temp);
		}

		/*
		 * Compute bishop mobility score:
		 */
		temp = pos.bishops[WHITE];
		while (temp)
		{
			const int square = _movegen.getMSB64( temp );
			score +=
				 pos.getBishopMobility(square, occupied);

			_movegen.clearBit64(square, temp);
		}

		temp = pos.bishops[BLACK];
		while (temp)
		{
			const int square = _movegen.getMSB64( temp );
			score -=
				 pos.getBishopMobility(square, occupied);

			_movegen.clearBit64(square, temp);
		}

		/*
		 * Compute queen mobility score:
		 */
		temp = pos.queens[WHITE];
		while (temp)
		{
			const int square = _movegen.getMSB64(temp);
			score +=
				 pos.getQueenMobility(square,occupied);

			_movegen.clearBit64(square, temp);
		}

		temp = pos.queens[BLACK];
		while (temp)
		{
			const int square = _movegen.getMSB64(temp);
			score -=
				 pos.getQueenMobility(square,occupied);

			_movegen.clearBit64(square, temp);
		}

		return
			score * WEIGHT;
	}

	uint32 evaluateKnights(const Position& pos) const
	{
		return 0;
	}

private:

	const MoveGen& _movegen;
};

/*
 * Note: This needs to correlate with the piece_t definition:
 */
const int Evaluator::piece_value[7] =
{
	INVALID      ,
	PAWN_VALUE   ,
	ROOK_VALUE   ,
	KNIGHT_VALUE ,
	BISHOP_VALUE ,
	QUEEN_VALUE  ,
	MATE_SCORE
};

#endif
