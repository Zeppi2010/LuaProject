#include <iostream>
#include <thread>
#include <string>
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <Windows.h>
#include "lua.hpp"
#include "raylib.h"
#include <entt.hpp>
#include <LuaBridge.h>
#include "states/GameState.h"
#include "states/IGameState.h"
#include "states/MainMenuState.h"
#include "states/GameplayState.h"
#include "states/EditorState.h"
#include "lua/LuaManager.h"

int main()
{
    // Shared ECS registry and Lua environment used by all states
    entt::registry registry;
    LuaManager luaManager(registry);
    luaManager.Init();  // opens Lua state and registers all ecs.* functions

    GameState current_state  = MAIN_MENU;
    GameState previous_state = MAIN_MENU;
    bool running = true;

    // All three states are created once and reused; switching calls Exit/Init on them
    MainMenuState mainMenu;
    GameplayState gameplay(registry, luaManager);
    EditorState   editor(luaManager.GetState());  // editor needs raw Lua state to load level files

    IGameState* activeState = &mainMenu;
    activeState->Init();

    InitWindow(900, 600, "Title");
    SetTargetFPS(60);   // cap at 60 so frame-counted timers are predictable
    SetExitKey(0);      // disable ESC closing the window (states handle navigation)

    while (!WindowShouldClose() && running)
    {
        activeState->Update(current_state);

        // Resume all Lua coroutines once per frame, after the state has run its systems
        luaManager.UpdateCoroutines();

        // Detect a state change, clean up the old state, and initialise the new one
        if (current_state != previous_state)
        {
            activeState->Exit();

            if (current_state == GAME)
                activeState = &gameplay;
            else if (current_state == EDITOR)
                activeState = &editor;
            else if (current_state == MAIN_MENU)
                activeState = &mainMenu;
            else if (current_state == QUIT)
                running = false;

            if (running)
                activeState->Init();

            previous_state = current_state;
        }
    }

    activeState->Exit();
    luaManager.Shutdown();
    CloseWindow();
    return 0;
}
