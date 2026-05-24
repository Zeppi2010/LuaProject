#pragma once
#include <entt.hpp>
#include "Components.h"
#include "raylib.h"

void RenderSystem(entt::registry& registry);
void MovementSystem(entt::registry& registry);
void InputSystem(entt::registry& registry);
void CollisionSystem(entt::registry& registry);
void AttackSystem(entt::registry& registry);
void FallDeathSystem(entt::registry& registry);
