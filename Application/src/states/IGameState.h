#pragma once
#include "GameState.h"

class IGameState
{
public:
	virtual void Init() = 0;
	virtual void Update(GameState& current_state) = 0;
	virtual void Exit() = 0;

	virtual ~IGameState() = default;

};