#pragma once
#include "lua.hpp"
#include "entt.hpp"

// World position of the entity (top-left corner)
struct TransformComponent
{
	float xPos;
	float yPos;
};

// Tracks current and max HP
struct HealthComponent
{
	int currentHP;
	int maxHP;
};

// How much damage an attack hitbox deals on contact
struct DamageComponent
{
	int damage;
};

// Movement speed in pixels per second on each axis
struct VelocityComponent
{
	float xV;
	float yV;
};

// Size of the entity's axis-aligned bounding box for collisions
struct CollisionComponent
{
	float width;
	float height;
};

// Holds the Lua coroutine thread and its registry reference for AI behaviour
struct BehaviourComponent
{
	lua_State* thread;
	int coRoutine;
	bool started = false;
};

// Enables gravity; tracks whether the entity is standing on a platform
struct PhysicsComponent
{
	bool isGrounded;
	float gravityScale;
};

// Attached to an active attack hitbox; counts down its lifetime and records its owner
struct AttackComponent
{
	int framesLeft;
	entt::entity owner;
	bool facingRight;
};

// Prevents the player from attacking again until the countdown reaches zero
struct AttackCooldownComponent
{
	int framesLeft;
};

// Tracks which direction the entity is facing, used for attack placement
struct FacingComponent
{
	bool facingRight = true;
};

// Records the last attack that hit this entity, prevents the same hitbox hitting twice
struct HitByComponent
{
	entt::entity attackEntity;
};

// Temporarily freezes the entity's AI and stops horizontal movement when grounded
struct StunnedComponent
{
	int framesLeft = 45;
};

// Makes the entity temporarily immune to damage; causes flashing on the player
struct InvincibilityComponent
{
	int framesLeft;
};

// Tag: marks this entity as the player
struct PlayerTagComponent {};

// Tag: marks this entity as an enemy
struct EnemyTagComponent {};

// Tag: marks this entity as a solid platform
struct PlatformTagComponent {};

// Tag: marks this entity as an active attack hitbox
struct AttackTagComponent {};

// Tag: marks this enemy as the boss (affects knockback, stun, and win condition)
struct BossTagComponent {};
