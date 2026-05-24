#include "GameplayState.h"

GameplayState::GameplayState(entt::registry& reg, LuaManager& lua) : registry(reg), luaManager(lua) {}

void GameplayState::Init()
{
    gameOver = false;
    gameOverTimer = 0;
    gameWon = false;
    gameWonTimer = 0;
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

    if (gameWon)
    {
        gameWonTimer--;
        if (gameWonTimer <= 0)
            current_state = MAIN_MENU;

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("YOU WIN!", 340, 260, 40, GREEN);
        DrawText("Returning to menu...", 310, 320, 20, WHITE);
        EndDrawing();
        return;
    }

    InputSystem(registry);
    AttackSystem(registry);
    CollisionSystem(registry);
    MovementSystem(registry);
    FallDeathSystem(registry);

    if (luaManager.HasWon())
    {
        gameWon = true;
        gameWonTimer = 180;
    }

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

    bool hasEnemies = !registry.view<EnemyTagComponent>().empty();
    if (!hasEnemies)
        DrawText("Room cleared!  Move right ->", 270, 30, 20, YELLOW);

    EndDrawing();
}

void GameplayState::Exit() {}
