#pragma once
#include "lua.hpp"
#include "entt.hpp"


struct TransformComponent
{
	float xPos;
	float yPos;
};

struct HealthComponent
{
	int currentHP;
	int maxHP;
};

struct DamageComponent
{
	int damage;
};

struct VelocityComponent
{
	float xV;
	float yV;
};

struct CollisionComponent
{
	float width;
	float height;
};

struct BehaviourComponent
{
	lua_State* thread;
	int coRoutine;
	bool started = false;
};

struct PhysicsComponent
{
	bool isGrounded;
	float gravityScale;
};

struct AttackComponent
{
	int framesLeft;
	entt::entity owner;
	bool facingRight;
};

struct AttackCooldownComponent
{
	int framesLeft;
};

struct FacingComponent
{
	bool facingRight = true;
};

struct HitByComponent
{
	entt::entity attackEntity;
};

struct StunnedComponent
{
	int framesLeft = 45;
};

struct InvincibilityComponent
{
	int framesLeft;
};

struct PlayerTagComponent
{

};

struct EnemyTagComponent
{

};

struct PlatformTagComponent
{

};

struct AttackTagComponent
{

};

struct BossTagComponent
{

};