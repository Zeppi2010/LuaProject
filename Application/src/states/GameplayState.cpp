#include "GameplayState.h"

GameplayState::GameplayState(entt::registry& reg, LuaManager& lua) : registry(reg), luaManager(lua) {}

// Clears all entities and reloads the level script so every playthrough starts fresh
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
    // R returns to main menu at any time
    if (IsKeyPressed(KEY_R))
    {
        current_state = MAIN_MENU;
        return;
    }

    // Show the game-over screen and return to menu after 3 seconds
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

    // Show the win screen and return to menu after 3 seconds
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

    // Run all gameplay systems in order each frame
    InputSystem(registry);
    AttackSystem(registry);
    CollisionSystem(registry);
    MovementSystem(registry);
    FallDeathSystem(registry);

    // Check whether the Lua room_manager triggered a win this frame
    if (luaManager.HasWon())
    {
        gameWon = true;
        gameWonTimer = 180;
    }

    // Check if the player's HP has dropped to zero
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

    // Hint text once the room is cleared but only while the player entity exists
    bool hasPlayer  = !registry.view<PlayerTagComponent>().empty();
    bool hasEnemies = !registry.view<EnemyTagComponent>().empty();
    if (hasPlayer && !hasEnemies)
        DrawText("Room cleared!  Move right ->", 270, 30, 20, YELLOW);

    EndDrawing();
}

void GameplayState::Exit() {}
