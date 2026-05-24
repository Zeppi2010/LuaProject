#pragma once
#include <string>
#include "lua.hpp"
#include <entt.hpp>
#include <LuaBridge.h>
#include "ecs/Components.h"
class LuaManager
{
	private:
		lua_State* L;
		entt::registry& registry;
		bool won = false;

	public:
		LuaManager(entt::registry& reg);
		void Init();
		void Reset();
		void Shutdown();
		void RunFile(const std::string& path);
		void RunString(const std::string& str);
		void UpdateCoroutines();
		bool HasWon() const { return won; }
		lua_State* GetState() const { return L; }

};