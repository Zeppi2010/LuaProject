#pragma once
#include "IGameState.h"
#include "raylib.h"

class EditorState : public IGameState
{
public:
	void Init();
	void Update(GameState& current_state);
	void Exit();
};