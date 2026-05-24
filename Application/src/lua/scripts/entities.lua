function enemy_patrol(entity_id)
    local ATTACK_RANGE = 50.0
    local WINDUP_FRAMES = 30
    local ATTACK_FRAMES = 15
    local COOLDOWN_FRAMES = 60

    while true do
        if not ecs.is_stunned(entity_id) then
            local px, py = ecs.get_player_position()
            local ex, ey = ecs.get_position(entity_id)
            local dist = math.abs(px - ex)

            if dist <= ATTACK_RANGE then
                ecs.set_velocity_x(entity_id, 0.0)
                for i = 1, WINDUP_FRAMES do
                    coroutine.yield()
                end
                ecs.create_enemy_attack(entity_id, 10)
                for i = 1, ATTACK_FRAMES do
                    coroutine.yield()
                end
                for i = 1, COOLDOWN_FRAMES do
                    coroutine.yield()
                end
            else
                if px > ex then
                    ecs.set_velocity_x(entity_id, 80.0)
                else
                    ecs.set_velocity_x(entity_id, -80.0)
                end
            end
        end
        coroutine.yield()
    end
end

function boss_behaviour(entity_id)
    local phase = 1
    local PHASE1_SPEED  = 120.0
    local PHASE1_DAMAGE = 15
    local PHASE1_RANGE  = 80.0
    local PHASE2_SPEED  = 100.0
    local PHASE2_DAMAGE = 20
    local PHASE2_RANGE  = 50.0

    while true do
        if not ecs.is_stunned(entity_id) then
            local hp, maxhp = ecs.get_health(entity_id)

            if phase == 1 and hp <= maxhp / 2 then
                phase = 2
                local ex, ey = ecs.get_position(entity_id)
                ecs.set_collision_size(entity_id, 30.0, 80.0)
                ecs.set_position(entity_id, ex + 15.0, ey - 40.0)
            end

            local px, py = ecs.get_player_position()
            local ex, ey = ecs.get_position(entity_id)
            local dist = math.abs(px - ex)

            local speed  = (phase == 1) and PHASE1_SPEED  or PHASE2_SPEED
            local damage = (phase == 1) and PHASE1_DAMAGE or PHASE2_DAMAGE
            local range  = (phase == 1) and PHASE1_RANGE  or PHASE2_RANGE

            if dist <= range then
                ecs.set_velocity_x(entity_id, 0.0)
                for i = 1, 30 do coroutine.yield() end
                ecs.create_enemy_attack(entity_id, damage)
                for i = 1, 15 do coroutine.yield() end
                for i = 1, 60 do coroutine.yield() end
            else
                if px > ex then
                    ecs.set_velocity_x(entity_id, speed)
                else
                    ecs.set_velocity_x(entity_id, -speed)
                end

                if phase == 2 and ecs.is_grounded(entity_id) and math.random(1, 120) == 1 then
                    ecs.set_velocity_y(entity_id, -400.0)
                end
            end
        end
        coroutine.yield()
    end
end

function game_manager(entity_id)
    local boss_spawned = false
    while true do
        if not boss_spawned and ecs.get_enemy_count() == 0 then
            boss_spawned = true
            local boss = ecs.create_entity()
            ecs.add_transform(boss, 600.0, 460.0)
            ecs.add_velocity(boss, 0.0, 0.0)
            ecs.add_physics(boss, 1.0)
            ecs.add_collision(boss, 60.0, 40.0)
            ecs.add_health(boss, 60)
            ecs.add_enemy_tag(boss)
            ecs.add_boss_tag(boss)
            ecs.add_behaviour(boss, "boss_behaviour")
        end
        coroutine.yield()
    end
end

local player = ecs.create_entity()
ecs.add_transform(player, 100.0, 200.0)
ecs.add_velocity(player, 0.0, 0.0)
ecs.add_player_tag(player)
ecs.add_physics(player, 1.0)
ecs.add_collision(player, 25.0, 75.0)
ecs.add_facing(player)
ecs.add_health(player, 100)

local platform = ecs.create_entity()
ecs.add_transform(platform, 0.0, 500.0)
ecs.add_collision(platform, 900.0, 20.0)
ecs.add_platform_tag(platform)

local enemy = ecs.create_entity()
ecs.add_transform(enemy, 400.0, 425.0)
ecs.add_velocity(enemy, 0.0, 0.0)
ecs.add_physics(enemy, 1.0)
ecs.add_collision(enemy, 25.0, 75.0)
ecs.add_health(enemy, 30)
ecs.add_enemy_tag(enemy)
ecs.add_behaviour(enemy, "enemy_patrol")

local manager = ecs.create_entity()
ecs.add_behaviour(manager, "game_manager")