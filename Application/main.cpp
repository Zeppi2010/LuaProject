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
    entt::registry registry;
    LuaManager luaManager(registry);
    luaManager.Init();

    GameState current_state = MAIN_MENU;
    GameState previous_state = MAIN_MENU;
    bool running = true;

    MainMenuState mainMenu;
    GameplayState gameplay(registry, luaManager);
    EditorState editor;

    IGameState* activeState = &mainMenu;
    activeState->Init();

    InitWindow(900, 600, "Title");
    SetExitKey(0);

    while (!WindowShouldClose() && running)
    {
        activeState->Update(current_state);
        luaManager.UpdateCoroutines();
        if (current_state != previous_state)
        {
            activeState->Exit();

            if (current_state == GAME) 
            {
                activeState = &gameplay;
            }
            else if (current_state == EDITOR)
            {
                activeState = &editor;
            }
            else if (current_state == MAIN_MENU)
            {
                activeState = &mainMenu;
            }
            else if (current_state == QUIT)
            {
                running = false;
            }

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