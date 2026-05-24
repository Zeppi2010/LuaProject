#pragma once
#include "IGameState.h"
#include "raylib.h"
#include <entt.hpp>
#include "ecs/Systems.h"
#include "lua/LuaManager.h"

class GameplayState : public IGameState
{
public:
    GameplayState(entt::registry& reg, LuaManager& lua);
    void Init() override;
    void Update(GameState& current_state) override;
    void Exit() override;
private:
    entt::registry& registry;
    LuaManager& luaManager;
    bool gameOver = false;
    int gameOverTimer = 0;
    bool gameWon = false;
    int gameWonTimer = 0;
};