#pragma once
#include "IGameState.h"
#include "raylib.h"
#include <entt.hpp>
#include "ecs/Systems.h"

class GameplayState : public IGameState
{
public:
    GameplayState(entt::registry& reg);
    void Init() override;
    void Update(GameState& current_state) override;
    void Exit() override;
private:
    entt::registry& registry;
};