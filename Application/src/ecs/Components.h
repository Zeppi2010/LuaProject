#pragma once
#include "lua.hpp" 
#include "raylib.h"
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

struct SpriteComponent
{
	Texture2D spriteTexture;
	Rectangle rectangle;
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
	int groundedFrames = 0;
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