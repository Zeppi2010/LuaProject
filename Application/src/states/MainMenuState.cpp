#include "MainMenuState.h"

void MainMenuState::Init()
{
}

void MainMenuState::Update(GameState& current_state)
{
	BeginDrawing();

	ClearBackground(BLUE);

	DrawText("MAIN MENU", 400, 300, 20, WHITE);
	DrawText("1 = Play", 400, 350, 20, WHITE);
	DrawText("2 = Editor", 400, 400, 20, WHITE);
	DrawText("3 = Quit", 400, 450, 20, WHITE);

	if (IsKeyPressed(KEY_ONE))
	{
		current_state = GAME;
	}
	else if (IsKeyPressed(KEY_TWO))
	{
		current_state = EDITOR;
	}
	else if (IsKeyPressed(KEY_THREE))
	{
		current_state = QUIT;
	}

	EndDrawing();
}

void MainMenuState::Exit()
{
}
