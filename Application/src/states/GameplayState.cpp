#include "GameplayState.h"

GameplayState::GameplayState(entt::registry& reg, LuaManager& lua) : registry(reg), luaManager(lua) {}

void GameplayState::Init()
{
    luaManager.Reset();
    luaManager.RunFile("src\\lua\\scripts\\entities.lua");
}

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