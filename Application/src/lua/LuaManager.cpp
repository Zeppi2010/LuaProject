#include "LuaManager.h"

LuaManager::LuaManager(entt::registry& reg) : registry(reg), L(nullptr) {}
void LuaManager::Init()
{
    L = luaL_newstate();
    luaL_openlibs(L);

    luabridge::getGlobalNamespace(L)
        .beginNamespace("ecs")
        .addFunction("create_entity", [this]() -> uint32_t
            {
                auto entity = registry.create();
                return (uint32_t)entity;
            })
        .addFunction("add_transform", [this](uint32_t id, float x, float y)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<TransformComponent>(entity, x, y);
                }
            })
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
        .addFunction("add_player_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<PlayerTagComponent>(entity);
                }
            })
        .addFunction("add_velocity", [this](uint32_t id, float xV, float yV)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<VelocityComponent>(entity, xV, yV);
                }
            })
        .addFunction("add_physics", [this](uint32_t id, float gravityScale)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<PhysicsComponent>(entity, false, gravityScale);;
                }
            })
        .addFunction("add_collision", [this](uint32_t id, float width, float height)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<CollisionComponent>(entity, width, height);
                }
            })
        .addFunction("add_platform_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<PlatformTagComponent>(entity);
                }
            })
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
        .addFunction("add_facing", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<FacingComponent>(entity);
                }
            })
        .addFunction("add_health", [this](uint32_t id, int hp)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<HealthComponent>(entity, hp, hp);
                }
            })
        .addFunction("add_enemy_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                {
                    registry.emplace<EnemyTagComponent>(entity);
                }
            })
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
        .addFunction("is_stunned", [this](uint32_t id) -> bool
            {
                auto entity = (entt::entity)id;
                return registry.valid(entity) && registry.all_of<StunnedComponent>(entity);
            })

        .addFunction("create_enemy_attack", [this](uint32_t id, int damage)
            {
                auto entity = (entt::entity)id;
                if (!registry.valid(entity) || !registry.all_of<TransformComponent, CollisionComponent>(entity)) return;

                auto& transform = registry.get<TransformComponent>(entity);
                auto& collision = registry.get<CollisionComponent>(entity);

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
        .addFunction("set_velocity_y", [this](uint32_t id, float yV)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<VelocityComponent>(entity))
                {
                    registry.get<VelocityComponent>(entity).yV = yV;
                }
            })
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
        .addFunction("is_grounded", [this](uint32_t id) -> bool
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<PhysicsComponent>(entity))
                    return registry.get<PhysicsComponent>(entity).isGrounded;
                return false;
            })
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
        .addFunction("add_boss_tag", [this](uint32_t id)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity))
                    registry.emplace<BossTagComponent>(entity);
            })
        .addFunction("set_velocity_x", [this](uint32_t id, float xV)
            {
                auto entity = (entt::entity)id;
                if (registry.valid(entity) && registry.all_of<VelocityComponent>(entity))
                {
                    auto& velocity = registry.get<VelocityComponent>(entity);
                    velocity.xV = xV;
                }
            })
        .endNamespace();
}
void LuaManager::Reset()
{
    auto view = registry.view<BehaviourComponent>();
    view.each([this](auto entity, BehaviourComponent& behaviour)
        {
            if (behaviour.coRoutine != LUA_NOREF && behaviour.coRoutine != LUA_REFNIL)
                luaL_unref(L, LUA_REGISTRYINDEX, behaviour.coRoutine);
        });
    registry.clear();
}
void LuaManager::Shutdown()
{
	if (L != nullptr)
	{
		lua_close(L);
		L = nullptr;
	}
}
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
void LuaManager::UpdateCoroutines()
{

    auto view = registry.view<BehaviourComponent>();
    view.each([this](auto entity, BehaviourComponent& behaviour)
        {
            if (behaviour.thread == nullptr) return;
            if (!registry.valid(entity)) return;

            int results = 0;
            int status = 0;
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
                // co-routinen pausade, forts�tt n�sta frame
            }
            else if (status == LUA_OK)
            {
                // co-routinen �r klar
                luaL_unref(L, LUA_REGISTRYINDEX, behaviour.coRoutine);
                behaviour.thread = nullptr;
            }
            else
            {
                // fel
                std::cerr << "Coroutine error: " << lua_tostring(behaviour.thread, -1) << std::endl;
                lua_pop(behaviour.thread, 1);
                behaviour.thread = nullptr;
            }
        });
}