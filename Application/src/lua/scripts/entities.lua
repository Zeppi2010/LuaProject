-- Patrol AI: walks toward the player, stops at platform edges,
-- and attacks when close enough; freezes while stunned
function enemy_patrol(entity_id)
    local ATTACK_RANGE    = 50.0
    local WINDUP_FRAMES   = 30   -- frames before the hitbox appears
    local ATTACK_FRAMES   = 15   -- frames the hitbox is active
    local COOLDOWN_FRAMES = 60   -- frames before the next attack
    local E_WIDTH  = 25.0
    local E_HEIGHT = 75.0

    while true do
        if not ecs.is_stunned(entity_id) then
            local px, py = ecs.get_player_position()
            local ex, ey = ecs.get_position(entity_id)
            local dist   = math.abs(px - ex)

            if dist <= ATTACK_RANGE then
                -- Player is close: stop, wind up, attack, then cool down
                ecs.set_velocity_x(entity_id, 0.0)
                for i = 1, WINDUP_FRAMES   do coroutine.yield() end
                ecs.create_enemy_attack(entity_id, 10)
                for i = 1, ATTACK_FRAMES   do coroutine.yield() end
                for i = 1, COOLDOWN_FRAMES do coroutine.yield() end
            else
                -- Check 2px past the leading edge just below the feet;
                -- stop if there is no platform there (edge detection)
                local ground_y = ey + E_HEIGHT + 2.0
                if px > ex then
                    if ecs.has_ground_at(ex + E_WIDTH + 2.0, ground_y) then
                        ecs.set_velocity_x(entity_id, 80.0)
                    else
                        ecs.set_velocity_x(entity_id, 0.0)
                    end
                else
                    if ecs.has_ground_at(ex - 2.0, ground_y) then
                        ecs.set_velocity_x(entity_id, -80.0)
                    else
                        ecs.set_velocity_x(entity_id, 0.0)
                    end
                end
            end
        end
        coroutine.yield()  -- pause until next frame
    end
end

-- Boss AI: two-phase behaviour.
-- Phase 1 (full HP): wide collision box, faster, 15 damage.
-- Phase 2 (below half HP): tall collision box, slightly slower, 20 damage,
--   random screen-crossing leaps when grounded.
function boss_behaviour(entity_id)
    local phase = 1
    local boss_width    = 60.0  -- tracks current collision width for gap calculation
    local PHASE1_SPEED  = 120.0
    local PHASE1_DAMAGE = 15
    local PHASE1_RANGE  = 20.0
    local PHASE2_SPEED  = 100.0
    local PHASE2_DAMAGE = 20
    local PHASE2_RANGE  = 20.0
    local PLAYER_WIDTH  = 25.0

    while true do
        -- Phase transition: resize collision and shift position so feet stay on ground
        local hp, maxhp = ecs.get_health(entity_id)
        if phase == 1 and hp <= maxhp / 2 then
            phase = 2
            boss_width = 30.0
            local ex, ey = ecs.get_position(entity_id)
            ecs.set_collision_size(entity_id, 30.0, 80.0)
            ecs.set_position(entity_id, ex + 15.0, ey - 40.0)
        end

        if not ecs.is_stunned(entity_id) then
            local px, py = ecs.get_player_position()
            local ex, ey = ecs.get_position(entity_id)

            -- Edge-to-edge gap so the range feels the same from both sides
            local gap
            if px >= ex then
                gap = math.max(0.0, px - (ex + boss_width))
            else
                gap = math.max(0.0, ex - (px + PLAYER_WIDTH))
            end

            local speed  = (phase == 1) and PHASE1_SPEED  or PHASE2_SPEED
            local damage = (phase == 1) and PHASE1_DAMAGE or PHASE2_DAMAGE
            local range  = (phase == 1) and PHASE1_RANGE  or PHASE2_RANGE

            if gap <= range then
                -- Attack sequence: wind up, release hitbox, cool down
                ecs.set_velocity_x(entity_id, 0.0)
                for i = 1, 30 do coroutine.yield() end
                ecs.create_enemy_attack(entity_id, damage)
                for i = 1, 15 do coroutine.yield() end
                for i = 1, 60 do coroutine.yield() end
            else
                -- Chase the player; phase 2 may randomly leap across the screen
                if px > ex then
                    ecs.set_velocity_x(entity_id, speed)
                else
                    ecs.set_velocity_x(entity_id, -speed)
                end

                if phase == 2 and ecs.is_grounded(entity_id) and math.random(1, 120) == 1 then
                    local leapX = (px > ex) and 480.0 or -480.0
                    ecs.set_velocity(entity_id, leapX, -400.0)
                end
            end
        end
        coroutine.yield()
    end
end

-- Room manager: loads room data from the editor-saved level file (or falls back
-- to built-in rooms), spawns the player, then handles room transitions and the
-- win condition every frame
function room_manager(entity_id)
    -- Try to load the editor's saved level; fall back to three hardcoded rooms
    local ok, level = pcall(dofile, "src/lua/scripts/level.lua")

    local rooms_data
    if ok and type(level) == "table" and level.rooms then
        rooms_data = level.rooms
    else
        rooms_data = {
            {
                spawn = { x=100, y=425 },
                platforms = {
                    {x=0,   y=500, w=300, h=20},
                    {x=350, y=420, w=200, h=20},
                    {x=600, y=500, w=300, h=20},
                },
                enemies = { {x=640, y=425, boss=false} },
            },
            {
                spawn = { x=100, y=425 },
                platforms = {
                    {x=0,   y=500, w=180, h=20},
                    {x=160, y=390, w=180, h=20},
                    {x=460, y=390, w=180, h=20},
                    {x=720, y=500, w=180, h=20},
                },
                enemies = {
                    {x=190, y=315, boss=false},
                    {x=470, y=315, boss=false},
                },
            },
            {
                spawn = { x=100, y=425 },
                platforms = { {x=0, y=500, w=900, h=20} },
                enemies   = { {x=600, y=460, boss=true} },
            },
        }
    end

    local num_rooms         = #rooms_data
    local current_platforms = {}  -- tracks IDs of platforms in the current room

    -- Create the player at the first room's spawn point
    local s1 = rooms_data[1].spawn
    local player = ecs.create_entity()
    ecs.add_transform(player, s1.x, s1.y)
    ecs.add_velocity(player, 0.0, 0.0)
    ecs.add_player_tag(player)
    ecs.add_physics(player, 1.0)
    ecs.add_collision(player, 25.0, 75.0)
    ecs.add_facing(player)
    ecs.add_health(player, 100)

    -- Removes all platform entities that belong to the current room
    local function clear_platforms()
        for _, pid in ipairs(current_platforms) do
            ecs.destroy_entity(pid)
        end
        current_platforms = {}
    end

    -- Spawns platforms and enemies defined for the given room index
    local function load_room(idx)
        local room = rooms_data[idx]
        for _, p in ipairs(room.platforms) do
            table.insert(current_platforms, ecs.add_platform(p.x, p.y, p.w, p.h))
        end
        for _, e in ipairs(room.enemies) do
            if e.boss then
                local boss = ecs.create_entity()
                ecs.add_transform(boss, e.x, e.y)
                ecs.add_velocity(boss, 0.0, 0.0)
                ecs.add_physics(boss, 1.0)
                ecs.add_collision(boss, 60.0, 40.0)
                ecs.add_health(boss, 60)
                ecs.add_enemy_tag(boss)
                ecs.add_boss_tag(boss)
                ecs.add_behaviour(boss, "boss_behaviour")
            else
                local en = ecs.create_entity()
                ecs.add_transform(en, e.x, e.y)
                ecs.add_velocity(en, 0.0, 0.0)
                ecs.add_physics(en, 1.0)
                ecs.add_collision(en, 25.0, 75.0)
                ecs.add_health(en, 30)
                ecs.add_enemy_tag(en)
                ecs.add_behaviour(en, "enemy_patrol")
            end
        end
    end

    load_room(1)
    local current = 1

    while true do
        if current == num_rooms then
            -- Last room: trigger win once all enemies and the boss are dead
            if ecs.get_enemy_count() == 0 and ecs.get_boss_count() == 0 then
                ecs.trigger_win()
            end
        else
            -- Earlier rooms: advance when the room is clear AND the player walks to the right edge
            local px, py = ecs.get_player_position()
            if ecs.get_enemy_count() == 0 and ecs.get_boss_count() == 0 and px >= 860.0 then
                current = current + 1
                clear_platforms()
                local sp = rooms_data[current].spawn
                ecs.reset_player_position(sp.x, sp.y)
                load_room(current)
            end
        end
        coroutine.yield()  -- check again next frame
    end
end

-- Create the manager entity that runs room_manager every frame
local manager = ecs.create_entity()
ecs.add_behaviour(manager, "room_manager")
