#pragma once
#include "Systems/stateMachine.hpp"

class MovementState : public StateMachine
{
public:
	MovementState();
	~MovementState();

	virtual void Start();
	virtual void Continue();
	virtual void End();

	virtual void StartTransition();
	virtual void EndTransition();
};

