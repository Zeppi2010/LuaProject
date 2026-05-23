function enemy_patrol(entity_id)
    while true do
        if not ecs.is_stunned(entity_id) then
            local px, py = ecs.get_player_position()
            local ex, ey = ecs.get_position(entity_id)
            if px > ex then
                ecs.set_velocity_x(entity_id, 80.0)
            else
                ecs.set_velocity_x(entity_id, -80.0)
            end
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