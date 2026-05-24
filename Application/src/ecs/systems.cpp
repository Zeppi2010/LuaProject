#include "systems.h"
#include <iostream>
#include <algorithm>
#include <string>

// Draws all platforms, characters, the player's HP bar, and active attack hitboxes
void RenderSystem(entt::registry& registry)
{
	// Draw every platform as a white rectangle
	auto platformView = registry.view<PlatformTagComponent, TransformComponent, CollisionComponent>();
	platformView.each([](auto entity, TransformComponent& transform, CollisionComponent& collision)
		{
			DrawRectangle((int)transform.xPos, (int)transform.yPos,
				(int)collision.width, (int)collision.height, WHITE);
		});

	// Draw every character (player, enemies); skip platforms and attack hitboxes
	auto characterView = registry.view<TransformComponent, CollisionComponent>();
	characterView.each([&](auto entity, TransformComponent& transform, CollisionComponent& collision)
		{
			if (registry.all_of<PlatformTagComponent>(entity) || registry.all_of<AttackTagComponent>(entity))
				return;

			// Flash the entity every 5 frames while invincible
			if (registry.all_of<InvincibilityComponent>(entity))
			{
				auto& inv = registry.get<InvincibilityComponent>(entity);
				if ((inv.framesLeft / 5) % 2 == 0) return;
			}

			Color color = registry.all_of<BossTagComponent>(entity) ? PURPLE : RED;
			DrawRectangle((int)transform.xPos, (int)transform.yPos,
				(int)collision.width, (int)collision.height, color);
		});

	// Draw the player's HP in the top-left corner
	auto hpView = registry.view<PlayerTagComponent, HealthComponent>();
	hpView.each([](auto entity, HealthComponent& health)
		{
			std::string hpText = "HP: " + std::to_string(health.currentHP) + " / " + std::to_string(health.maxHP);
			DrawText(hpText.c_str(), 10, 10, 20, WHITE);
		});

	// Draw active attack hitboxes in yellow so the player can see them
	auto attackView = registry.view<AttackTagComponent, TransformComponent, CollisionComponent>();
	attackView.each([](auto entity, TransformComponent& transform, CollisionComponent& collision)
		{
			DrawRectangle((int)transform.xPos, (int)transform.yPos,
				(int)collision.width, (int)collision.height, YELLOW);
		});
}

// Applies gravity, moves every entity by its velocity, ticks down status timers,
// and stops entities from leaving the screen horizontally
void MovementSystem(entt::registry& registry)
{
	const float GRAVITY = 500.0f;
	float dt = GetFrameTime();

	auto transView = registry.view<TransformComponent, VelocityComponent>();
	auto physView  = registry.view<PhysicsComponent, VelocityComponent>();

	// Stun: zero horizontal speed while grounded; count down and remove when done
	auto stunView = registry.view<StunnedComponent, VelocityComponent, PhysicsComponent>();
	stunView.each([&](auto entity, StunnedComponent& stun, VelocityComponent& velocity, PhysicsComponent& physics)
		{
			if (physics.isGrounded)
				velocity.xV = 0.0f;
			stun.framesLeft--;
			if (stun.framesLeft <= 0)
				registry.remove<StunnedComponent>(entity);
		});

	// Invincibility: count down and remove when expired
	auto invincView = registry.view<InvincibilityComponent>();
	invincView.each([&](auto entity, InvincibilityComponent& inv)
		{
			inv.framesLeft--;
			if (inv.framesLeft <= 0)
				registry.remove<InvincibilityComponent>(entity);
		});

	// Attack cooldown: count down and remove when expired
	auto cooldownView = registry.view<AttackCooldownComponent>();
	cooldownView.each([&](auto entity, AttackCooldownComponent& cd)
		{
			cd.framesLeft--;
			if (cd.framesLeft <= 0)
				registry.remove<AttackCooldownComponent>(entity);
		});

	// Accumulate downward velocity for entities not standing on anything
	physView.each([GRAVITY, dt](auto entity, PhysicsComponent& physics, VelocityComponent& velocity)
		{
			if (!physics.isGrounded)
				velocity.yV += GRAVITY * physics.gravityScale * dt;
		});

	// Move every entity by its current velocity
	transView.each([dt](auto entity, TransformComponent& transform, VelocityComponent& velocity)
		{
			transform.xPos += velocity.xV * dt;
			transform.yPos += velocity.yV * dt;
		});

	// Clamp physics entities to the screen width, zeroing velocity on impact
	const float SCREEN_WIDTH = 900.0f;
	auto boundsView = registry.view<TransformComponent, CollisionComponent, PhysicsComponent, VelocityComponent>();
	boundsView.each([SCREEN_WIDTH](auto entity, TransformComponent& transform, CollisionComponent& collision, PhysicsComponent&, VelocityComponent& velocity)
		{
			if (transform.xPos < 0.0f)
			{
				transform.xPos = 0.0f;
				if (velocity.xV < 0.0f) velocity.xV = 0.0f;
			}
			if (transform.xPos + collision.width > SCREEN_WIDTH)
			{
				transform.xPos = SCREEN_WIDTH - collision.width;
				if (velocity.xV > 0.0f) velocity.xV = 0.0f;
			}
		});
}

// Reads keyboard input and moves the player; preserves knockback while invincible
void InputSystem(entt::registry& registry)
{
	auto view = registry.view<PlayerTagComponent, VelocityComponent, PhysicsComponent, FacingComponent>();
	view.each([&](auto entity, VelocityComponent& velocity, PhysicsComponent& physics, FacingComponent& facing)
		{
			// Only reset horizontal speed when not in knockback (no invincibility frames)
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
				velocity.yV = -350.0f;
		});
}

// Checks every physics entity against every platform; lands the entity on top
// of any platform it falls into from above
void CollisionSystem(entt::registry& registry)
{
	auto physicsView  = registry.view<PhysicsComponent, TransformComponent, CollisionComponent, VelocityComponent>();
	auto platformView = registry.view<PlatformTagComponent, TransformComponent, CollisionComponent>();

	physicsView.each([&](auto entity, PhysicsComponent& physics, TransformComponent& transform,
	                     CollisionComponent& collision, VelocityComponent& velocity)
		{
			physics.isGrounded = false;

			float playerLeft   = transform.xPos;
			float playerRight  = transform.xPos + collision.width;
			float playerTop    = transform.yPos;
			float playerBottom = transform.yPos + collision.height;

			platformView.each([&](auto platEntity, TransformComponent& platTransform,
			                      CollisionComponent& platCollision)
				{
					float platLeft   = platTransform.xPos;
					float platRight  = platTransform.xPos + platCollision.width;
					float platTop    = platTransform.yPos;
					float platBottom = platTransform.yPos + platCollision.height;

					bool overlaps = playerRight > platLeft && playerLeft < platRight &&
					                playerBottom > platTop && playerTop < platBottom;

					// Only resolve when falling into the platform from above
					if (overlaps && velocity.yV > 0)
					{
						physics.isGrounded = true;
						velocity.yV = 0;
						transform.yPos = platTop - collision.height;
					}
				});
		});
}

// Spawns player attack hitboxes on SPACE, moves each hitbox with its owner,
// deals damage on overlap, and destroys hitboxes when their lifetime expires
void AttackSystem(entt::registry& registry)
{
	// Spawn a new player attack hitbox when SPACE is pressed and not on cooldown
	auto playerView = registry.view<PlayerTagComponent, TransformComponent, CollisionComponent, FacingComponent>();
	playerView.each([&](auto playerEntity, TransformComponent& transform, CollisionComponent& collision, FacingComponent& facing)
		{
			if (IsKeyPressed(KEY_SPACE) && !registry.all_of<AttackCooldownComponent>(playerEntity))
			{
				float attackX = facing.facingRight ?
					transform.xPos + collision.width :
					transform.xPos - 30.0f;
				float attackY = transform.yPos + (collision.height - 40.0f) / 2.0f;
				auto attack = registry.create();
				registry.emplace<TransformComponent>(attack, attackX, attackY);
				registry.emplace<CollisionComponent>(attack, 30.0f, 40.0f);
				registry.emplace<AttackTagComponent>(attack);
				registry.emplace<DamageComponent>(attack, 10);
				registry.emplace<AttackComponent>(attack, 15, playerEntity, facing.facingRight);
				registry.emplace<AttackCooldownComponent>(playerEntity, 25);
			}
		});

	std::vector<entt::entity> toDestroy;

	auto attackView = registry.view<AttackTagComponent, TransformComponent, CollisionComponent, AttackComponent, DamageComponent>();
	attackView.each([&](auto attackEntity, TransformComponent& atkTransform, CollisionComponent& atkCollision,
	                    AttackComponent& attack, DamageComponent& damage)
		{
			// Snap the hitbox to its owner every frame so it follows movement
			if (registry.valid(attack.owner) && registry.all_of<TransformComponent, CollisionComponent>(attack.owner))
			{
				auto& ownerT = registry.get<TransformComponent>(attack.owner);
				auto& ownerC = registry.get<CollisionComponent>(attack.owner);
				atkTransform.xPos = attack.facingRight ?
					ownerT.xPos + ownerC.width :
					ownerT.xPos - atkCollision.width;
				atkTransform.yPos = ownerT.yPos + (ownerC.height - 40.0f) / 2.0f;
			}

			attack.framesLeft--;

			bool ownerIsEnemy = registry.valid(attack.owner) && registry.all_of<EnemyTagComponent>(attack.owner);

			if (ownerIsEnemy)
			{
				// Enemy attack: check overlap with player, apply damage and knockback
				auto playerView = registry.view<PlayerTagComponent, TransformComponent, CollisionComponent, HealthComponent, VelocityComponent>();
				playerView.each([&](auto playerEntity, TransformComponent& pTransform, CollisionComponent& pCollision,
				                    HealthComponent& health, VelocityComponent& pVelocity)
					{
						if (registry.all_of<InvincibilityComponent>(playerEntity)) return;
						// Ignore if the player was already hit by this specific hitbox
						if (registry.all_of<HitByComponent>(playerEntity))
						{
							auto& hitBy = registry.get<HitByComponent>(playerEntity);
							if (hitBy.attackEntity == attackEntity) return;
						}

						float atkLeft = atkTransform.xPos, atkRight = atkTransform.xPos + atkCollision.width;
						float atkTop  = atkTransform.yPos, atkBottom = atkTransform.yPos + atkCollision.height;
						float pLeft   = pTransform.xPos,   pRight  = pTransform.xPos + pCollision.width;
						float pTop    = pTransform.yPos,   pBottom = pTransform.yPos + pCollision.height;

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
				// Player attack: check overlap with each enemy, apply damage and knockback
				auto enemyView = registry.view<EnemyTagComponent, TransformComponent, CollisionComponent, HealthComponent, VelocityComponent>();
				enemyView.each([&](auto enemyEntity, TransformComponent& enemyTransform, CollisionComponent& enemyCollision,
				                   HealthComponent& health, VelocityComponent& enemyVelocity)
					{
						float atkLeft = atkTransform.xPos, atkRight = atkTransform.xPos + atkCollision.width;
						float atkTop  = atkTransform.yPos, atkBottom = atkTransform.yPos + atkCollision.height;
						float eLeft   = enemyTransform.xPos, eRight  = enemyTransform.xPos + enemyCollision.width;
						float eTop    = enemyTransform.yPos, eBottom = enemyTransform.yPos + enemyCollision.height;

						bool overlaps = atkRight > eLeft && atkLeft < eRight &&
						                atkBottom > eTop && atkTop < eBottom;
						if (overlaps)
						{
							if (registry.all_of<InvincibilityComponent>(enemyEntity)) return;

							bool isBoss = registry.all_of<BossTagComponent>(enemyEntity);
							registry.emplace_or_replace<HitByComponent>(enemyEntity, attackEntity);
							// Boss gets longer invincibility; regular enemies also get stunned
							registry.emplace_or_replace<InvincibilityComponent>(enemyEntity, isBoss ? 60 : 30);
							if (!isBoss)
								registry.emplace_or_replace<StunnedComponent>(enemyEntity, 45);
							health.currentHP -= damage.damage;
							// Boss receives less knockback so it doesn't slide across the screen
							float knockbackX = isBoss ? 80.0f : 200.0f;
							enemyVelocity.xV = (enemyTransform.xPos > atkTransform.xPos) ? knockbackX : -knockbackX;
							enemyVelocity.yV = -150.0f;

							if (health.currentHP <= 0)
								toDestroy.push_back(enemyEntity);
						}
					});
			}

			if (attack.framesLeft <= 0)
				toDestroy.push_back(attackEntity);
		});

	// Destroy expired hitboxes and dead enemies after the iteration is done
	for (auto e : toDestroy)
		if (registry.valid(e))
			registry.destroy(e);
}

// Destroys enemies that fall below the screen; sets the player's HP to zero
// so the existing game-over logic triggers if the player falls off
void FallDeathSystem(entt::registry& registry)
{
	const float DEATH_Y = 620.0f;

	std::vector<entt::entity> toDestroy;
	auto enemyView = registry.view<EnemyTagComponent, TransformComponent>();
	enemyView.each([&](auto entity, TransformComponent& t)
		{
			if (t.yPos > DEATH_Y)
				toDestroy.push_back(entity);
		});
	for (auto e : toDestroy)
		if (registry.valid(e))
			registry.destroy(e);

	// Don't destroy the player entity; let the HP-zero check in GameplayState handle it
	auto playerView = registry.view<PlayerTagComponent, TransformComponent, HealthComponent>();
	playerView.each([](auto entity, TransformComponent& t, HealthComponent& health)
		{
			if (t.yPos > DEATH_Y)
				health.currentHP = 0;
		});
}
