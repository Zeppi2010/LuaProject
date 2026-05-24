#include "EditorState.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <string>

static const char* LEVEL_PATH = "src/lua/scripts/level.lua";

EditorState::EditorState(lua_State* L) : L(L) {}

void EditorState::Init()
{
    rooms.clear();
    currentRoom = 0;
    tool        = EdTool::Platform;
    dragging    = false;
    savedFlash  = false;
    savedTimer  = 0;

    LoadFromFile();

    if (rooms.empty())
    {
        EdRoom r;
        r.platforms.push_back({0, 500, 900, 20});
        rooms.push_back(r);
    }
}

void EditorState::Exit() {}

float EditorState::Snap(float v) const
{
    return std::round(v / GRID) * GRID;
}

bool EditorState::Button(float x, float y, float w, float h,
                          const char* label, bool active)
{
    Rectangle r = {x, y, w, h};
    Vector2   m = GetMousePosition();
    bool hovered = CheckCollisionPointRec(m, r);
    Color bg = active   ? Color{0, 80, 180, 255}
             : hovered  ? Color{90, 90, 90, 255}
                        : Color{55, 55, 55, 255};
    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, 1, LIGHTGRAY);
    int tw = MeasureText(label, 14);
    DrawText(label, (int)(x + w / 2 - tw / 2), (int)(y + h / 2 - 7), 14, WHITE);
    return hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

// ---------- Canvas drawing ----------

void EditorState::DrawCanvas()
{
    // Background
    DrawRectangle(0, 0, 900, (int)CANVAS_H, Color{28, 28, 28, 255});

    // Grid
    for (int gx = 0; gx < 900; gx += (int)GRID)
        DrawLine(gx, 0, gx, (int)CANVAS_H, Color{42, 42, 42, 255});
    for (int gy = 0; gy < (int)CANVAS_H; gy += (int)GRID)
        DrawLine(0, gy, 900, gy, Color{42, 42, 42, 255});

    if (currentRoom >= (int)rooms.size()) return;
    auto& room = rooms[currentRoom];

    // Platforms
    for (auto& p : room.platforms)
    {
        DrawRectangle((int)p.x, (int)p.y, (int)p.w, (int)p.h, WHITE);
        DrawRectangleLinesEx({p.x, p.y, p.w, p.h}, 1, DARKGRAY);
    }

    // Enemies / boss
    for (auto& e : room.enemies)
    {
        float ew = e.isBoss ? 60.0f : 25.0f;
        float eh = e.isBoss ? 40.0f : 75.0f;
        Color c  = e.isBoss ? PURPLE : RED;
        DrawRectangle((int)e.x, (int)e.y, (int)ew, (int)eh, c);
        DrawRectangleLinesEx({e.x, e.y, ew, eh}, 1, WHITE);
        const char* lbl = e.isBoss ? "B" : "E";
        int lw = MeasureText(lbl, 16);
        DrawText(lbl, (int)(e.x + ew / 2 - lw / 2), (int)(e.y + eh / 2 - 8), 16, WHITE);
    }

    // Spawn
    {
        DrawRectangle((int)room.spawnX, (int)room.spawnY, 25, 75, Color{0, 180, 0, 200});
        DrawRectangleLinesEx({room.spawnX, room.spawnY, 25, 75}, 1, GREEN);
        DrawText("S", (int)(room.spawnX + 8), (int)(room.spawnY + 28), 16, WHITE);
    }

    // Preview at cursor
    Vector2 raw = GetMousePosition();
    if (raw.y >= 0 && raw.y < CANVAS_H && raw.x >= 0 && raw.x < 900)
    {
        float sx = Snap(raw.x), sy = Snap(raw.y);
        if (tool == EdTool::Platform)
        {
            if (dragging)
            {
                float px = std::min(sx, dragX);
                float py = std::min(sy, dragY);
                float pw = std::abs(sx - dragX);
                float ph = std::abs(sy - dragY);
                DrawRectangle((int)px, (int)py, (int)pw, (int)ph,
                              Color{200, 200, 200, 60});
                DrawRectangleLinesEx({px, py, pw, ph}, 1, WHITE);
            }
            else
            {
                DrawRectangleLinesEx({sx, sy, 100, 20}, 1,
                                     Color{200, 200, 200, 160});
            }
        }
        else if (tool == EdTool::Enemy)
        {
            DrawRectangle((int)sx, (int)(sy - 75), 25, 75, Color{220, 50, 50, 90});
            DrawRectangleLinesEx({sx, sy - 75, 25, 75}, 1, RED);
        }
        else if (tool == EdTool::Boss)
        {
            DrawRectangle((int)sx, (int)(sy - 40), 60, 40, Color{180, 0, 220, 90});
            DrawRectangleLinesEx({sx, sy - 40, 60, 40}, 1, PURPLE);
        }
        else if (tool == EdTool::Spawn)
        {
            DrawRectangle((int)sx, (int)(sy - 75), 25, 75, Color{0, 200, 0, 90});
            DrawRectangleLinesEx({sx, sy - 75, 25, 75}, 1, GREEN);
        }
    }
}

// ---------- Toolbar ----------

void EditorState::DrawToolbar(GameState& current_state)
{
    DrawRectangle(0, (int)TB_Y, 900, (int)TB_H, Color{20, 20, 20, 255});
    DrawLine(0, (int)TB_Y, 900, (int)TB_Y, LIGHTGRAY);

    float ty = TB_Y + 5;
    float th = TB_H - 10;

    // Tools
    if (Button(4,   ty, 82, th, "Platform", tool == EdTool::Platform)) tool = EdTool::Platform;
    if (Button(90,  ty, 65, th, "Enemy",    tool == EdTool::Enemy))    tool = EdTool::Enemy;
    if (Button(159, ty, 55, th, "Boss",     tool == EdTool::Boss))     tool = EdTool::Boss;
    if (Button(218, ty, 60, th, "Spawn",    tool == EdTool::Spawn))    tool = EdTool::Spawn;

    // Room navigation
    if (Button(295, ty, 22, th, "<"))
        if (currentRoom > 0) currentRoom--;

    std::string roomLabel = "Room " + std::to_string(currentRoom + 1)
                          + " / " + std::to_string((int)rooms.size());
    int rlw = MeasureText(roomLabel.c_str(), 14);
    DrawText(roomLabel.c_str(), (int)(360 - rlw / 2), (int)(TB_Y + 17), 14, WHITE);

    if (Button(428, ty, 22, th, ">"))
        if (currentRoom < (int)rooms.size() - 1) currentRoom++;

    if (Button(456, ty, 62, th, "+ Room"))
    {
        EdRoom r;
        r.platforms.push_back({0, 500, 900, 20});
        rooms.insert(rooms.begin() + currentRoom + 1, r);
        currentRoom++;
    }
    if (Button(522, ty, 62, th, "- Room"))
    {
        if ((int)rooms.size() > 1)
        {
            rooms.erase(rooms.begin() + currentRoom);
            if (currentRoom >= (int)rooms.size()) currentRoom--;
        }
    }

    // Last room label
    if (currentRoom == (int)rooms.size() - 1)
    {
        DrawText("[Boss room]", 597, (int)(TB_Y + 17), 13, YELLOW);
    }

    // Save / Menu
    if (savedFlash)
    {
        DrawText("Saved!", 710, (int)(TB_Y + 17), 14, GREEN);
        if (--savedTimer <= 0) savedFlash = false;
    }

    if (Button(740, ty, 65, th, "Save"))
    {
        Save();
        savedFlash = true;
        savedTimer = 120;
    }
    if (Button(812, ty, 82, th, "Menu"))
        current_state = MAIN_MENU;
}

// ---------- Main update ----------

void EditorState::Update(GameState& current_state)
{
    Vector2 raw     = GetMousePosition();
    float sx        = Snap(raw.x);
    float sy        = Snap(raw.y);
    bool  inCanvas  = (raw.y >= 0 && raw.y < CANVAS_H && raw.x >= 0 && raw.x < 900.0f);

    // Platform drag: release anywhere (cancel if outside canvas)
    if (dragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
    {
        dragging = false;
        if (inCanvas)
        {
            float px = std::min(sx, dragX), py = std::min(sy, dragY);
            float pw = std::abs(sx - dragX), ph = std::abs(sy - dragY);
            if (pw >= MIN_PLW && ph >= MIN_PLH)
                rooms[currentRoom].platforms.push_back({px, py, pw, ph});
        }
    }

    if (inCanvas && currentRoom < (int)rooms.size())
    {
        auto& room = rooms[currentRoom];

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (tool == EdTool::Platform)
            {
                dragging = true;
                dragX = sx; dragY = sy;
            }
            else if (tool == EdTool::Enemy)
            {
                room.enemies.push_back({sx, sy - 75.0f, false});
            }
            else if (tool == EdTool::Boss)
            {
                room.enemies.push_back({sx, sy - 40.0f, true});
            }
            else if (tool == EdTool::Spawn)
            {
                room.spawnX = sx;
                room.spawnY = sy - 75.0f;
            }
        }

        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
            TryDelete(sx, sy);
    }

    BeginDrawing();
    DrawCanvas();
    DrawToolbar(current_state);
    EndDrawing();
}

// ---------- Delete ----------

void EditorState::TryDelete(float sx, float sy)
{
    auto& room = rooms[currentRoom];

    // Enemies first (rendered on top)
    for (int i = (int)room.enemies.size() - 1; i >= 0; i--)
    {
        auto& e = room.enemies[i];
        float ew = e.isBoss ? 60.0f : 25.0f;
        float eh = e.isBoss ? 40.0f : 75.0f;
        if (sx >= e.x && sx <= e.x + ew && sy >= e.y && sy <= e.y + eh)
        {
            room.enemies.erase(room.enemies.begin() + i);
            return;
        }
    }

    // Platforms
    for (int i = (int)room.platforms.size() - 1; i >= 0; i--)
    {
        auto& p = room.platforms[i];
        if (sx >= p.x && sx <= p.x + p.w && sy >= p.y && sy <= p.y + p.h)
        {
            room.platforms.erase(room.platforms.begin() + i);
            return;
        }
    }
}

// ---------- Save ----------

void EditorState::Save()
{
    std::ofstream f(LEVEL_PATH);
    if (!f.is_open()) return;

    f << "return {\n";
    f << "  rooms = {\n";
    for (auto& room : rooms)
    {
        f << "    {\n";
        f << "      spawn = { x = " << room.spawnX << ", y = " << room.spawnY << " },\n";
        f << "      platforms = {\n";
        for (auto& p : room.platforms)
            f << "        { x=" << p.x << ", y=" << p.y
              << ", w=" << p.w << ", h=" << p.h << " },\n";
        f << "      },\n";
        f << "      enemies = {\n";
        for (auto& e : room.enemies)
            f << "        { x=" << e.x << ", y=" << e.y
              << ", boss=" << (e.isBoss ? "true" : "false") << " },\n";
        f << "      },\n";
        f << "    },\n";
    }
    f << "  },\n";
    f << "}\n";
}

// ---------- Load ----------

void EditorState::LoadFromFile()
{
    if (luaL_loadfile(L, LEVEL_PATH) != LUA_OK)
    {
        lua_pop(L, 1);
        return;
    }
    if (lua_pcall(L, 0, 1, 0) != LUA_OK)
    {
        lua_pop(L, 1);
        return;
    }
    if (lua_istable(L, -1))
        ParseTable();
    lua_pop(L, 1);
}

void EditorState::ParseTable()
{
    // stack: [level_table]
    lua_getfield(L, -1, "rooms");
    if (!lua_istable(L, -1)) { lua_pop(L, 1); return; }

    int n = (int)lua_rawlen(L, -1);
    for (int i = 1; i <= n; i++)
    {
        lua_rawgeti(L, -1, i);
        EdRoom room;

        // spawn
        lua_getfield(L, -1, "spawn");
        if (lua_istable(L, -1))
        {
            lua_getfield(L, -1, "x"); room.spawnX = (float)lua_tonumber(L, -1); lua_pop(L, 1);
            lua_getfield(L, -1, "y"); room.spawnY = (float)lua_tonumber(L, -1); lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // platforms
        lua_getfield(L, -1, "platforms");
        if (lua_istable(L, -1))
        {
            int np = (int)lua_rawlen(L, -1);
            for (int j = 1; j <= np; j++)
            {
                lua_rawgeti(L, -1, j);
                EdPlatform p{};
                lua_getfield(L, -1, "x"); p.x = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lua_getfield(L, -1, "y"); p.y = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lua_getfield(L, -1, "w"); p.w = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lua_getfield(L, -1, "h"); p.h = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                room.platforms.push_back(p);
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        // enemies
        lua_getfield(L, -1, "enemies");
        if (lua_istable(L, -1))
        {
            int ne = (int)lua_rawlen(L, -1);
            for (int k = 1; k <= ne; k++)
            {
                lua_rawgeti(L, -1, k);
                EdEnemy e{};
                lua_getfield(L, -1, "x");    e.x      = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lua_getfield(L, -1, "y");    e.y      = (float)lua_tonumber(L, -1); lua_pop(L, 1);
                lua_getfield(L, -1, "boss"); e.isBoss = lua_toboolean(L, -1) != 0;  lua_pop(L, 1);
                room.enemies.push_back(e);
                lua_pop(L, 1);
            }
        }
        lua_pop(L, 1);

        rooms.push_back(room);
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop rooms table
}
