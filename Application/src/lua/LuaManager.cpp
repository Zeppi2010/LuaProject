#include "LuaManager.h"

LuaManager::LuaManager(entt::registry& reg) : registry(reg), L(nullptr) {}

// Opens a new Lua state and registers all ECS functions under the "ecs" namespace
// so Lua scripts can create and modify entities
void LuaManager::Init()
{
    L = luaL_newstate();
    luaL_openlibs(L);

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ecs")

        // Creates a new blank entity and returns its numeric ID
        .addFunction("create_entity", [this]() -> uint32_t
            {
                auto entity = registry.create();
                return (uint32_t)entity;
            })

        // Gives an entity a world position
        .addFunction("add_transform", [this](uint32_t id, float x, float y)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<TransformComponent>(entity, x, y);
            })

        // Attaches a Lua coroutine to an entity; the coroutine is resumed each frame
        .addFunction("add_behaviour", [this](uint32_t id, std::string funcName)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    lua_State* thread = lua_newthread(L);
                    lua_getglobal(thread, funcName.c_str());
                    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
                    registry.emplace<BehaviourComponent>(entity, thread, ref);
                }
            })

        // Tags the entity as the player (used by systems to identify it)
        .addFunction("add_player_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<PlayerTagComponent>(entity);
            })

        // Gives an entity an initial velocity
        .addFunction("add_velocity", [this](uint32_t id, float xV, float yV)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<VelocityComponent>(entity, xV, yV);
            })

        // Enables gravity on the entity, scaled by gravityScale
        .addFunction("add_physics", [this](uint32_t id, float gravityScale)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<PhysicsComponent>(entity, false, gravityScale);
            })

        // Gives an entity an AABB used for collision detection
        .addFunction("add_collision", [this](uint32_t id, float width, float height)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<CollisionComponent>(entity, width, height);
            })

        // Tags the entity as a solid platform that other entities land on
        .addFunction("add_platform_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<PlatformTagComponent>(entity);
            })

        // Overwrites the entity's current velocity
        .addFunction("set_velocity", [this](uint32_t id, float xV, float yV)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    auto& velocity = registry.get<VelocityComponent>(entity);
                    velocity.xV = xV;
                    velocity.yV = yV;
                }
            })

        // Stores which direction the entity faces; used when spawning attack hitboxes
        .addFunction("add_facing", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<FacingComponent>(entity);
            })

        // Adds a health component with both current and max HP set to hp
        .addFunction("add_health", [this](uint32_t id, int hp)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<HealthComponent>(entity, hp, hp);
            })

        // Tags the entity as an enemy (used by attack routing and win conditions)
        .addFunction("add_enemy_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<EnemyTagComponent>(entity);
            })

        // Returns the player's world position so enemy AI can navigate toward it
        .addFunction("get_player_position", [this]() -> std::tuple<float, float>
            {
                float px = 0.0f, py = 0.0f;
                auto view = registry.view<PlayerTagComponent, TransformComponent>();
                view.each([&](auto entity, TransformComponent& transform)
                    {
                        px = transform.xPos;
                        py = transform.yPos;
                    });
                return std::make_tuple(px, py);
            })

        // Returns the world position of any entity by its ID
        .addFunction("get_position", [this](uint32_t id) -> std::tuple<float, float>
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<TransformComponent>(entity))
                {
                    auto& transform = registry.get<TransformComponent>(entity);
                    return std::make_tuple(transform.xPos, transform.yPos);
                }
                return std::make_tuple(0.0f, 0.0f);
            })

        // Returns true if the entity currently has a StunnedComponent
        .addFunction("is_stunned", [this](uint32_t id) -> bool
            {
                auto entity = (entt::entity)id;
                return registry.valid(entity) && registry.all_of<StunnedComponent>(entity);
            })

        // Spawns a short-lived attack hitbox in front of the enemy, aimed at the player
        .addFunction("create_enemy_attack", [this](uint32_t id, int damage)
            {
                auto entity = (entt::entity)id;
                if (!registry.valid(entity) || !registry.all_of<TransformComponent, CollisionComponent>(entity)) return;

                auto& transform = registry.get<TransformComponent>(entity);
                auto& collision = registry.get<CollisionComponent>(entity);

                // Find the player's x to decide which side to place the hitbox
                float px = transform.xPos;
                auto playerView = registry.view<PlayerTagComponent, TransformComponent>();
                playerView.each([&](auto e, TransformComponent& t) { px = t.xPos; });

                bool attackRight = px > transform.xPos;
                float attackX = attackRight ? transform.xPos + collision.width : transform.xPos - 30.0f;
                float attackY = transform.yPos + (collision.height - 40.0f) / 2.0f;

                auto attack = registry.create();
                registry.emplace<TransformComponent>(attack, attackX, attackY);
                registry.emplace<CollisionComponent>(attack, 30.0f, 40.0f);
                registry.emplace<AttackTagComponent>(attack);
                registry.emplace<DamageComponent>(attack, damage);
                registry.emplace<AttackComponent>(attack, 15, entity, attackRight);
            })

        // Sets only the vertical velocity; used for jump attacks and leaps
        .addFunction("set_velocity_y", [this](uint32_t id, float yV)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<VelocityComponent>(entity))
                    registry.get<VelocityComponent>(entity).yV = yV;
            })

        // Teleports an entity to the given position
        .addFunction("set_position", [this](uint32_t id, float x, float y)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<TransformComponent>(entity))
                {
                    auto& t = registry.get<TransformComponent>(entity);
                    t.xPos = x;
                    t.yPos = y;
                }
            })

        // Resizes an entity's collision box; used when the boss changes phase
        .addFunction("set_collision_size", [this](uint32_t id, float width, float height)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<CollisionComponent>(entity))
                {
                    auto& c = registry.get<CollisionComponent>(entity);
                    c.width = width;
                    c.height = height;
                }
            })

        // Returns the entity's current and max HP
        .addFunction("get_health", [this](uint32_t id) -> std::tuple<int, int>
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<HealthComponent>(entity))
                {
                    auto& h = registry.get<HealthComponent>(entity);
                    return std::make_tuple(h.currentHP, h.maxHP);
                }
                return std::make_tuple(0, 0);
            })

        // Returns true if the entity is currently standing on a platform
        .addFunction("is_grounded", [this](uint32_t id) -> bool
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<PhysicsComponent>(entity))
                    return registry.get<PhysicsComponent>(entity).isGrounded;
                return false;
            })

        // Counts non-boss enemies; used to check if a room is cleared
        .addFunction("get_enemy_count", [this]() -> int
            {
                int count = 0;
                auto view = registry.view<EnemyTagComponent>();
                view.each([&](auto entity)
                    {
                        if (!registry.all_of<BossTagComponent>(entity))
                            count++;
                    });
                return count;
            })

        // Tags the entity as a boss (different knockback, no stun, longer i-frames)
        .addFunction("add_boss_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<BossTagComponent>(entity);
            })

        // Returns the number of living boss entities; used for the win condition
        .addFunction("get_boss_count", [this]() -> int
            {
                return (int)registry.view<BossTagComponent>().size();
            })

        // Moves the player entity to a new position and zeroes its velocity;
        // used when transitioning between rooms
        .addFunction("reset_player_position", [this](float x, float y)
            {
                auto view = registry.view<PlayerTagComponent, TransformComponent, VelocityComponent>();
                view.each([x, y](auto entity, TransformComponent& t, VelocityComponent& v)
                    {
                        t.xPos = x; t.yPos = y;
                        v.xV = 0.0f; v.yV = 0.0f;
                    });
            })

        // Sets the win flag; GameplayState reads this flag to show the win screen
        .addFunction("trigger_win", [this]()
            {
                won = true;
            })

        // Sets only the horizontal velocity; used by enemy patrol and boss movement
        .addFunction("set_velocity_x", [this](uint32_t id, float xV)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<VelocityComponent>(entity))
                    registry.get<VelocityComponent>(entity).xV = xV;
            })

        // Creates a solid platform entity and returns its ID so it can be removed later
        .addFunction("add_platform", [this](float x, float y, float width, float height) -> uint32_t
            {
                auto entity = registry.create();
                registry.emplace<TransformComponent>(entity, x, y);
                registry.emplace<CollisionComponent>(entity, width, height);
                registry.emplace<PlatformTagComponent>(entity);
                return (uint32_t)entity;
            })

        // Destroys any entity by ID; used by room_manager to clear platforms between rooms
        .addFunction("destroy_entity", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.destroy(entity);
            })

        // Returns true if any platform covers the point (x, y); used for enemy edge detection
        .addFunction("has_ground_at", [this](float x, float y) -> bool
            {
                auto view = registry.view<PlatformTagComponent, TransformComponent, CollisionComponent>();
                for (auto entity : view)
                {
                    auto& t = registry.get<TransformComponent>(entity);
                    auto& c = registry.get<CollisionComponent>(entity);
                    if (x >= t.xPos && x <= t.xPos + c.width &&
                        y >= t.yPos && y <= t.yPos + c.height)
                        return true;
                }
                return false;
            })

        .endNamespace();
}

// Unrefs all coroutine handles from the Lua registry, clears every entity,
// and resets the win flag so the game can start fresh
void LuaManager::Reset()
{
    auto view = registry.view<BehaviourComponent>();
    view.each([this](auto entity, BehaviourComponent& behaviour)
        {
            if (behaviour.coRoutine != LUA_NOREF && behaviour.coRoutine != LUA_REFNIL)
                luaL_unref(L, LUA_REGISTRYINDEX, behaviour.coRoutine);
        });
    registry.clear();
    won = false;
}

// Closes the Lua state and frees all Lua memory
void LuaManager::Shutdown()
{
	if (L != nullptr)
	{
		lua_close(L);
		L = nullptr;
	}
}

// Loads and runs a Lua file; prints any error to the console
void LuaManager::RunFile(const std::string& path)
{
	if (luaL_dofile(L, path.c_str()) != LUA_OK)
	{
		if (lua_gettop(L) && lua_isstring(L, -1))
		{
			std::cout << "Lua Error: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}
}

// Runs a Lua string directly; used for quick one-off commands
void LuaManager::RunString(const std::string& str)
{
	if (luaL_dostring(L, str.c_str()) != LUA_OK)
	{
		if (lua_gettop(L) && lua_isstring(L, -1))
		{
			std::cout << "Lua Error: " << lua_tostring(L, -1) << std::endl;
			lua_pop(L, 1);
		}
	}
}

// Iterates every entity that has a BehaviourComponent and resumes its Lua coroutine
// once per frame; the coroutine can yield to pause until the next frame
void LuaManager::UpdateCoroutines()
{
    auto view = registry.view<BehaviourComponent>();
    view.each([this](auto entity, BehaviourComponent& behaviour)
        {
            if (behaviour.thread == nullptr) return;
            if (!registry.valid(entity)) return;

            int results = 0;
            int status  = 0;

            // First resume: pass the entity ID as the first argument to the coroutine function
            if (!behaviour.started)
            {
                lua_pushinteger(behaviour.thread, (uint32_t)entity);
                behaviour.started = true;
                status = lua_resume(behaviour.thread, nullptr, 1, &results);
            }
            else
            {
                status = lua_resume(behaviour.thread, nullptr, 0, &results);
            }

            if (status == LUA_YIELD)
            {
                // Coroutine paused with coroutine.yield(); will resume next frame
            }
            else if (status == LUA_OK)
            {
                // Coroutine returned normally; clean up its registry reference
                luaL_unref(L, LUA_REGISTRYINDEX, behaviour.coRoutine);
                behaviour.thread = nullptr;
            }
            else
            {
                // Coroutine hit an error; print it and stop running the coroutine
                std::cerr << "Coroutine error: " << lua_tostring(behaviour.thread, -1) << std::endl;
                lua_pop(behaviour.thread, 1);
                behaviour.thread = nullptr;
            }
        });
}
