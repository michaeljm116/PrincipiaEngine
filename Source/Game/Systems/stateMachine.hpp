#pragma once
class StateMachine
{
public:
	StateMachine() {};
	~StateMachine() {};

	virtual void Start() = 0;
	virtual void Continue() = 0;
	virtual void End() = 0;

	virtual void StartTransition() = 0;
	virtual void EndTransition() = 0;
};

