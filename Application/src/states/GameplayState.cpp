#include "GameplayState.h"

GameplayState::GameplayState(entt::registry& reg) : registry(reg) {}

void GameplayState::Init() {}

void GameplayState::Update(GameState& current_state)
{
    if (IsKeyPressed(KEY_R))
    {
        current_state = MAIN_MENU;
    }
    InputSystem(registry);
    AttackSystem(registry);
    CollisionSystem(registry);
    MovementSystem(registry);
    BeginDrawing();
    ClearBackground(BLUE);
    RenderSystem(registry);
    EndDrawing();
}

void GameplayState::Exit() {}