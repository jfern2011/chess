#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "StateMachine.h"

class ChessEngine
{

public:

	ChessEngine();

	~ChessEngine();

	bool init();

	bool run();

private:

	/**
	 * The state machine, which determines what the program is doing
	 * at any given time
	 */
	StateMachine _state_machine;
};

#endif