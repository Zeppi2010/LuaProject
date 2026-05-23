#include "systems.h"
#include <iostream>

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
			if (!registry.all_of<PlatformTagComponent>(entity) && !registry.all_of<AttackTagComponent>(entity))
			{
				DrawRectangle((int)transform.xPos, (int)transform.yPos,
					(int)collision.width, (int)collision.height, RED);
			}
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
				velocity.xV = 0;  // ? nollst�ll bara n�r p� marken
				stun.groundedFrames++;
				if (stun.groundedFrames >= 120)
					registry.remove<StunnedComponent>(entity);
			}
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
	view.each([](auto entity, VelocityComponent& velocity, PhysicsComponent& physics, FacingComponent& facing)
		{
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
			if (IsKeyPressed(KEY_SPACE))
			{
				auto attack = registry.create();
				float attackX = facing.facingRight ?
					transform.xPos + collision.width :
					transform.xPos - 30.0f;
				registry.emplace<TransformComponent>(attack, attackX, transform.yPos + 20.0f);
				registry.emplace<CollisionComponent>(attack, 30.0f, 40.0f);
				registry.emplace<AttackTagComponent>(attack);
				registry.emplace<DamageComponent>(attack, 10);
				registry.emplace<AttackComponent>(attack, 15, playerEntity);
			}
		});

	// hantera aktiva attacker
	std::vector<entt::entity> toDestroy;
	auto attackView = registry.view<AttackTagComponent, TransformComponent, CollisionComponent, AttackComponent, DamageComponent>();
	attackView.each([&](auto attackEntity, TransformComponent& atkTransform, CollisionComponent& atkCollision, AttackComponent& attack, DamageComponent& damage)
		{
			attack.framesLeft--;

			// kolla kollision mot fiender
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
						// kolla om fienden redan tr�ffats av denna attack
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
						{
							toDestroy.push_back(enemyEntity);
						}
					}
				});

			if (attack.framesLeft <= 0)
			{
				toDestroy.push_back(attackEntity);
			}
		});

	for (auto e : toDestroy)
	{
		if (registry.valid(e))
			registry.destroy(e);
	}
}