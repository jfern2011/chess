#ifndef __STATE_MACHINE__
#define __STATE_MACHINE__

class StateMachine
{

public:

	StateMachine()
	{
	}

	virtual ~StateMachine()
	{
	}

private:
	
};

class xBoardStateMachine : public StateMachine
{

public:

	typedef enum
	{
		READY,
		FORCE

	} state_t;

	xBoardStateMachine()
		: _current_state(READY)
	{
	}

	~xBoardStateMachine()
	{
	}

	state_t getState() const
	{
		return
			_current_state;
	}

	bool updateState(state_t state)
	{
		_current_state = state;
		return true;
	}

private:

	state_t
		_current_state;
};

#endif