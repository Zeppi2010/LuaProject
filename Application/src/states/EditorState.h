#pragma once
#include "IGameState.h"
#include "GameState.h"
#include "raylib.h"
#include "lua.hpp"
#include <vector>
#include <string>

struct EdPlatform { float x, y, w, h; };
struct EdEnemy    { float x, y; bool isBoss; };

struct EdRoom
{
    std::vector<EdPlatform> platforms;
    std::vector<EdEnemy>    enemies;
    float spawnX = 100.0f;
    float spawnY = 425.0f;
};

enum class EdTool { Platform, Enemy, Boss, Spawn };

class EditorState : public IGameState
{
public:
    explicit EditorState(lua_State* L);
    void Init()  override;
    void Update(GameState& current_state) override;
    void Exit()  override;

private:
    lua_State* L;
    std::vector<EdRoom> rooms;
    int     currentRoom = 0;
    EdTool  tool        = EdTool::Platform;
    bool    dragging    = false;
    float   dragX = 0, dragY = 0;
    bool    savedFlash  = false;
    int     savedTimer  = 0;

    static constexpr float CANVAS_H  = 550.0f;
    static constexpr float TB_Y      = 550.0f;
    static constexpr float TB_H      = 50.0f;
    static constexpr float GRID      = 10.0f;
    static constexpr float MIN_PLW   = 20.0f;
    static constexpr float MIN_PLH   = 10.0f;

    float   Snap(float v) const;
    bool    Button(float x, float y, float w, float h,
                   const char* label, bool active = false);
    void    DrawCanvas();
    void    DrawToolbar(GameState& current_state);
    void    TryDelete(float sx, float sy);
    void    Save();
    void    LoadFromFile();
    void    ParseTable();
};
