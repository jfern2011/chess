#ifndef __EVAL_H__
#define __EVAL_H__

#include "movegen.h"

#define SCORE_INF 1000000

class Evaluator
{
	typedef enum
	{
		PAWN_VALUE   = 1000,
		KNIGHT_VALUE = 3250,
		BISHOP_VALUE = 3250,
		ROOK_VALUE   = 5000,
		QUEEN_VALUE  = 9750

	} value_t;

public:

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

#endif