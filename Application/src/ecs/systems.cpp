#include "systems.h"
#include <iostream>
#include <algorithm>
#include <string>

void RenderSystem(entt::registry& registry)
{
	// rita platforms
	auto platformView = registry.view<PlatformTagComponent, TransformComponent, CollisionComponent>();
	platformView.each([](auto entity, TransformComponent& transform, CollisionComponent& collision)
		{
			DrawRectangle((int)transform.xPos, (int)transform.yPos,
				(int)collision.width, (int)collision.height, WHITE);
		});

	auto characterView = registry.view<TransformComponent, CollisionComponent>();
	characterView.each([&](auto entity, TransformComponent& transform, CollisionComponent& collision)
		{
			if (registry.all_of<PlatformTagComponent>(entity) || registry.all_of<AttackTagComponent>(entity))
				return;

			if (registry.all_of<InvincibilityComponent>(entity))
			{
				auto& inv = registry.get<InvincibilityComponent>(entity);
				if ((inv.framesLeft / 5) % 2 == 0) return;
			}

			DrawRectangle((int)transform.xPos, (int)transform.yPos,
				(int)collision.width, (int)collision.height, RED);
		});

	auto hpView = registry.view<PlayerTagComponent, HealthComponent>();
	hpView.each([](auto entity, HealthComponent& health)
		{
			std::string hpText = "HP: " + std::to_string(health.currentHP) + " / " + std::to_string(health.maxHP);
			DrawText(hpText.c_str(), 10, 10, 20, WHITE);
		});

	auto attackView = registry.view<AttackTagComponent, TransformComponent, CollisionComponent>();
	attackView.each([](auto entity, TransformComponent& transform, CollisionComponent& collision)
		{
			DrawRectangle((int)transform.xPos, (int)transform.yPos,
				(int)collision.width, (int)collision.height, YELLOW);
		});
}

void MovementSystem(entt::registry& registry)
{
	const float GRAVITY = 500.0f;
	float dt = GetFrameTime();
	auto transView = registry.view<TransformComponent, VelocityComponent>();
	auto physView = registry.view<PhysicsComponent, VelocityComponent>();
	auto stunView = registry.view<StunnedComponent, VelocityComponent, PhysicsComponent>();
	stunView.each([&](auto entity, StunnedComponent& stun, VelocityComponent& velocity, PhysicsComponent& physics)
		{
			if (physics.isGrounded)
			{
				velocity.xV = 0;
				stun.groundedFrames++;
				if (stun.groundedFrames >= 120)
					registry.remove<StunnedComponent>(entity);
			}
		});

	auto invincView = registry.view<InvincibilityComponent>();
	invincView.each([&](auto entity, InvincibilityComponent& inv)
		{
			inv.framesLeft--;
			if (inv.framesLeft <= 0)
				registry.remove<InvincibilityComponent>(entity);
		});

	auto cooldownView = registry.view<AttackCooldownComponent>();
	cooldownView.each([&](auto entity, AttackCooldownComponent& cd)
		{
			cd.framesLeft--;
			if (cd.framesLeft <= 0)
				registry.remove<AttackCooldownComponent>(entity);
		});
	physView.each([GRAVITY, dt](auto entity, PhysicsComponent& physics, VelocityComponent& velocity)
		{
			if (!physics.isGrounded)
			{
				velocity.yV += GRAVITY * physics.gravityScale * dt;
			}
		});

	transView.each([dt](auto entity, TransformComponent& transform, VelocityComponent& velocity)
		{
			transform.xPos += velocity.xV * dt;
			transform.yPos += velocity.yV * dt;
		});
}

void InputSystem(entt::registry& registry)
{
	auto view = registry.view<PlayerTagComponent, VelocityComponent, PhysicsComponent, FacingComponent>();
	view.each([&](auto entity, VelocityComponent& velocity, PhysicsComponent& physics, FacingComponent& facing)
		{
			if (!registry.all_of<InvincibilityComponent>(entity))
				velocity.xV = 0;
			if (IsKeyDown(KEY_A))
			{
				facing.facingRight = false;
				velocity.xV = -150.0f;
			}
			if (IsKeyDown(KEY_D))
			{
				facing.facingRight = true;
				velocity.xV = 150.0f;

			}
			if (IsKeyDown(KEY_W) && physics.isGrounded)
			{
				velocity.yV = -350.0f;
			}
		});
}

void CollisionSystem(entt::registry& registry)
{
	auto physicsView = registry.view<PhysicsComponent, TransformComponent, CollisionComponent, VelocityComponent>();
	auto platformView = registry.view<PlatformTagComponent, TransformComponent, CollisionComponent>();

	physicsView.each([&](auto entity, PhysicsComponent& physics, TransformComponent& transform, CollisionComponent& collision, VelocityComponent& velocity)
		{
			physics.isGrounded = false;

			float playerLeft = transform.xPos;
			float playerRight = transform.xPos + collision.width;
			float playerTop = transform.yPos;
			float playerBottom = transform.yPos + collision.height;

			platformView.each([&](auto platEntity, TransformComponent& platTransform, CollisionComponent& platCollision)
				{
					float platLeft = platTransform.xPos;
					float platRight = platTransform.xPos + platCollision.width;
					float platTop = platTransform.yPos;
					float platBottom = platTransform.yPos + platCollision.height;

					bool overlaps = playerRight > platLeft && playerLeft < platRight &&
						playerBottom > platTop && playerTop < platBottom;

					if (overlaps && velocity.yV > 0)
					{
						physics.isGrounded = true;
						velocity.yV = 0;
						transform.yPos = platTop - collision.height;
					}
				});
		});


}
void AttackSystem(entt::registry& registry)
{
	// skapa attack n�r SPACE trycks
	auto playerView = registry.view<PlayerTagComponent, TransformComponent, CollisionComponent, FacingComponent>();
	playerView.each([&](auto playerEntity, TransformComponent& transform, CollisionComponent& collision, FacingComponent& facing)
		{
			if (IsKeyPressed(KEY_SPACE) && !registry.all_of<AttackCooldownComponent>(playerEntity))
			{
				float attackX = facing.facingRight ?
					transform.xPos + collision.width :
					transform.xPos - 30.0f;
				auto attack = registry.create();
				registry.emplace<TransformComponent>(attack, attackX, transform.yPos + 20.0f);
				registry.emplace<CollisionComponent>(attack, 30.0f, 40.0f);
				registry.emplace<AttackTagComponent>(attack);
				registry.emplace<DamageComponent>(attack, 10);
				registry.emplace<AttackComponent>(attack, 15, playerEntity, facing.facingRight);
				registry.emplace<AttackCooldownComponent>(playerEntity, 25);
			}
		});

	std::vector<entt::entity> toDestroy;
	auto attackView = registry.view<AttackTagComponent, TransformComponent, CollisionComponent, AttackComponent, DamageComponent>();
	attackView.each([&](auto attackEntity, TransformComponent& atkTransform, CollisionComponent& atkCollision, AttackComponent& attack, DamageComponent& damage)
		{
			if (registry.valid(attack.owner) && registry.all_of<TransformComponent, CollisionComponent>(attack.owner))
			{
				auto& ownerT = registry.get<TransformComponent>(attack.owner);
				auto& ownerC = registry.get<CollisionComponent>(attack.owner);
				atkTransform.xPos = attack.facingRight ?
					ownerT.xPos + ownerC.width :
					ownerT.xPos - atkCollision.width;
				atkTransform.yPos = ownerT.yPos + 20.0f;
			}

			attack.framesLeft--;

			bool ownerIsEnemy = registry.valid(attack.owner) && registry.all_of<EnemyTagComponent>(attack.owner);

			if (ownerIsEnemy)
			{
				auto playerView = registry.view<PlayerTagComponent, TransformComponent, CollisionComponent, HealthComponent, VelocityComponent>();
				playerView.each([&](auto playerEntity, TransformComponent& pTransform, CollisionComponent& pCollision, HealthComponent& health, VelocityComponent& pVelocity)
					{
						if (registry.all_of<InvincibilityComponent>(playerEntity)) return;
						if (registry.all_of<HitByComponent>(playerEntity))
						{
							auto& hitBy = registry.get<HitByComponent>(playerEntity);
							if (hitBy.attackEntity == attackEntity) return;
						}

						float atkLeft = atkTransform.xPos;
						float atkRight = atkTransform.xPos + atkCollision.width;
						float atkTop = atkTransform.yPos;
						float atkBottom = atkTransform.yPos + atkCollision.height;

						float pLeft = pTransform.xPos;
						float pRight = pTransform.xPos + pCollision.width;
						float pTop = pTransform.yPos;
						float pBottom = pTransform.yPos + pCollision.height;

						bool overlaps = atkRight > pLeft && atkLeft < pRight &&
							atkBottom > pTop && atkTop < pBottom;

						if (overlaps)
						{
							registry.emplace_or_replace<HitByComponent>(playerEntity, attackEntity);
							registry.emplace_or_replace<InvincibilityComponent>(playerEntity, 60);
							health.currentHP = std::max(0, health.currentHP - damage.damage);
							pVelocity.xV = (pTransform.xPos > atkTransform.xPos) ? 250.0f : -250.0f;
							pVelocity.yV = -200.0f;
						}
					});
			}
			else
			{
				auto enemyView = registry.view<EnemyTagComponent, TransformComponent, CollisionComponent, HealthComponent, VelocityComponent>();
				enemyView.each([&](auto enemyEntity, TransformComponent& enemyTransform, CollisionComponent& enemyCollision, HealthComponent& health, VelocityComponent& enemyVelocity)
					{
						float atkLeft = atkTransform.xPos;
						float atkRight = atkTransform.xPos + atkCollision.width;
						float atkTop = atkTransform.yPos;
						float atkBottom = atkTransform.yPos + atkCollision.height;

						float eLeft = enemyTransform.xPos;
						float eRight = enemyTransform.xPos + enemyCollision.width;
						float eTop = enemyTransform.yPos;
						float eBottom = enemyTransform.yPos + enemyCollision.height;

						bool overlaps = atkRight > eLeft && atkLeft < eRight &&
							atkBottom > eTop && atkTop < eBottom;

						if (overlaps)
						{
							if (registry.all_of<HitByComponent>(enemyEntity))
							{
								auto& hitBy = registry.get<HitByComponent>(enemyEntity);
								if (hitBy.attackEntity == attackEntity) return;
							}

							registry.emplace_or_replace<HitByComponent>(enemyEntity, attackEntity);
							registry.emplace_or_replace<StunnedComponent>(enemyEntity);
							health.currentHP -= damage.damage;
							enemyVelocity.xV = (enemyTransform.xPos > atkTransform.xPos) ? 200.0f : -200.0f;
							enemyVelocity.yV = -150.0f;

							if (health.currentHP <= 0)
								toDestroy.push_back(enemyEntity);
						}
					});
			}

			if (attack.framesLeft <= 0)
				toDestroy.push_back(attackEntity);
		});

	for (auto e : toDestroy)
	{
		if (registry.valid(e))
			registry.destroy(e);
	}
}