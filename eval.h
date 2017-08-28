#ifndef __EVAL_H__
#define __EVAL_H__

#include "movegen.h"

class Evaluator
{

public:

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
		return
			pos.material[WHITE] - pos.material[BLACK];
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
