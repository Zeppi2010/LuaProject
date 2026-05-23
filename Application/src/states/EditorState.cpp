#include "EditorState.h"

void EditorState::Init() {}
void EditorState::Update(GameState& current_state)
{
    if (IsKeyPressed(KEY_R))
    {
        current_state = MAIN_MENU;
    }
    BeginDrawing();
    ClearBackground(BLUE);
    DrawText("Editor", 450, 300, 20, WHITE);
    EndDrawing();
}
void EditorState::Exit() {}