#include "GameplayState.h"

GameplayState::GameplayState(entt::registry& reg, LuaManager& lua) : registry(reg), luaManager(lua) {}

void GameplayState::Init()
{
    gameOver = false;
    gameOverTimer = 0;
    luaManager.Reset();
    luaManager.RunFile("src\\lua\\scripts\\entities.lua");
}

void GameplayState::Update(GameState& current_state)
{
    if (IsKeyPressed(KEY_R))
    {
        current_state = MAIN_MENU;
        return;
    }

    if (gameOver)
    {
        gameOverTimer--;
        if (gameOverTimer <= 0)
            current_state = MAIN_MENU;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("GAME OVER", 310, 260, 40, RED);
        DrawText("Returning to menu...", 310, 320, 20, WHITE);
        EndDrawing();
        return;
    }

    InputSystem(registry);
    AttackSystem(registry);
    CollisionSystem(registry);
    MovementSystem(registry);

    auto playerView = registry.view<PlayerTagComponent, HealthComponent>();
    playerView.each([&](auto entity, HealthComponent& health)
        {
            if (health.currentHP <= 0)
            {
                gameOver = true;
                gameOverTimer = 180;
            }
        });

    BeginDrawing();
    ClearBackground(BLUE);
    RenderSystem(registry);
    EndDrawing();
}

void GameplayState::Exit() {}